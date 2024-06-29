inair = true

function on_grounded(force)
    entity.transform:set_rot(mat4.rotate({0, 1, 0}, math.random()*360))
    inair = false
end

function on_fall()
    inair = true
end

function on_update()
    local tsf = entity.transform
    local body = entity.rigidbody
    if inair then
        tsf:set_rot(mat4.rotate(tsf:get_rot(), {0, 1, 0}, math.random()*12))
        tsf:set_rot(mat4.rotate(tsf:get_rot(), {0, 0, 1}, math.random()*12))
    end
    local dir = vec3.sub({player.get_pos(hud.get_player())}, tsf:get_pos())
    vec3.normalize(dir, dir)
    vec3.mul(dir, time.delta()*50.0, dir)
    --body:set_vel(vec3.add(rigidbody:get_vel(), dir))
end
