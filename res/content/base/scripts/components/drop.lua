local tsf = entity.transform
local body = entity.rigidbody

inair = true
ready = false

function on_grounded(force)
    tsf:set_rot(mat4.rotate({0, 1, 0}, math.random()*360))
    inair = false
    ready = true
end

function on_fall()
    inair = true
end

function on_trigger_enter(index, oid)
    if ready and oid == 0 then
        entity:despawn()
    end
end

function on_update()
    if inair then
        local dt = time.delta();
        tsf:set_rot(mat4.rotate(tsf:get_rot(), {0, 1, 0}, 240*dt))
        tsf:set_rot(mat4.rotate(tsf:get_rot(), {0, 0, 1}, 240*dt))
    end
end
