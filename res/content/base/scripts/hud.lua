local DROP_FORCE = 8
local DROP_INIT_VEL = {0, 3, 0}

function on_hud_open()
    input.add_callback("player.drop", function ()
        local pid = hud.get_player()
        local pvel = {player.get_vel(pid)}
        local eid = entity.spawn("base:drop")
        local throw_force = vec3.mul(player.get_dir(pid), DROP_FORCE)
        entity.set_vel(eid, vec3.add(throw_force, vec3.add(pvel, DROP_INIT_VEL)))
        entity.set_rot(eid, mat4.rotate({0, 1, 0}, math.random() * 360))
    end)
end
