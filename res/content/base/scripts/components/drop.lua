local tsf = entity.transform
local body = entity.rigidbody
local rig = entity.skeleton

inair = true
target = -1
timer = 0.3

local dropitem = ARGS
if dropitem then 
    timer = dropitem.pickup_delay or timer
end
if SAVED_DATA.item then
    dropitem.id = item.index(SAVED_DATA.item)
    dropitem.count = SAVED_DATA.count
end

local DROP_SCALE = 0.3
local scale = {1, 1, 1}
local rotation = mat4.rotate({
    math.random(), math.random(), math.random()
}, 360)

function on_save()
    SAVED_DATA.item = item.name(dropitem.id)
    SAVED_DATA.count = dropitem.count
end

do -- setup visuals
    local matrix = mat4.idt()
    rig:set_model(0, item.model_name(dropitem.id))
    local bodysize = math.min(scale[1], scale[2], scale[3]) * DROP_SCALE
    body:set_size({scale[1] * DROP_SCALE, bodysize, scale[3] * DROP_SCALE})
    mat4.mul(matrix, rotation, matrix)
    mat4.scale(matrix, scale, matrix)
    rig:set_matrix(0, matrix)
end

function on_grounded(force)
    local matrix = mat4.idt()
    mat4.rotate(matrix, {0, 1, 0}, math.random()*360, matrix)
    mat4.rotate(matrix, {1, 0, 0}, 90, matrix)
    mat4.scale(matrix, scale, matrix)
    rig:set_matrix(0, matrix)
    inair = false
end

function on_fall()
    inair = true
end

function on_sensor_enter(index, oid)
    local playerid = hud.get_player()
    local playerentity = player.get_entity(playerid)
    if timer < 0.0 and oid == playerentity and index == 0 then
        entity:despawn()
        inventory.add(player.get_inventory(playerid), dropitem.id, dropitem.count)
        audio.play_sound_2d("events/pickup", 0.5, 0.8+math.random()*0.4, "regular")
    end
    if index == 1 and oid == playerentity then
        target = oid
    end
end

function on_sensor_exit(index, oid)
    if oid == target and index == 1 then
        target = -1
    end
end

function on_render()
    if inair then
        local dt = time.delta();

        mat4.rotate(rotation, {0, 1, 0}, 240*dt, rotation)
        mat4.rotate(rotation, {0, 0, 1}, 240*dt, rotation)

        local matrix = mat4.idt()
        mat4.mul(matrix, rotation, matrix)
        mat4.scale(matrix, scale, matrix)
        rig:set_matrix(0, matrix)
    end
end

function on_update(tps)
    timer = timer - 1.0/tps
    if target ~= -1 then
        if timer > 0.0 then
            return
        end
        local dir = vec3.sub(entities.get(target).transform:get_pos(), tsf:get_pos())
        vec3.normalize(dir, dir)
        vec3.mul(dir, 10.0, dir)
        body:set_vel(dir)
    end
end

function on_attacked(attacker, pid)
    body:set_vel({0, 10, 0})
end
