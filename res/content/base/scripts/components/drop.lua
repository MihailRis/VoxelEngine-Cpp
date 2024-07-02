local tsf = entity.transform
local body = entity.rigidbody
local rig = entity.modeltree

inair = true
ready = false

local item = ARGS.item

local rotation = mat4.rotate({0, 1, 0}, math.random() * 360)
mat4.rotate(rotation, {1, 0, 0}, math.random() * 360, rotation)
mat4.rotate(rotation, {0, 0, 1}, math.random() * 360, rotation)
rig:set_matrix(0, rotation)

function on_grounded(force)
    rig:set_matrix(0, mat4.rotate({0, 1, 0}, math.random()*360))
    inair = false
    ready = true
end

function on_fall()
    inair = true
end

function on_trigger_enter(index, oid)
    if ready and oid == 0 then
        entity:despawn()
        inventory.add(player.get_inventory(oid), item.id, item.count)
        audio.play_sound_2d("events/pickup", 0.5, 0.8+math.random()*0.4, "regular")
    end
end

function on_update()
    if inair then
        local dt = time.delta();
        local matrix = rig:get_matrix(0)
        mat4.rotate(matrix, {0, 1, 0}, 240*dt, matrix)
        mat4.rotate(matrix, {0, 0, 1}, 240*dt, matrix)
        rig:set_matrix(0, matrix)
    end
end
