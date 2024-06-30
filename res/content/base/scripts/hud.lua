local DROP_FORCE = 8
local DROP_INIT_VEL = {0, 3, 0}

function on_hud_open()
    input.add_callback("player.drop", function ()
        for i=1,80 do
        local pid = hud.get_player()
        local pvel = {player.get_vel(pid)}
        local ppos = vec3.add({player.get_pos(pid)}, {0, 0.7, 0})
        local throw_force = vec3.mul(vec3.add(player.get_dir(pid), 
        {
            (math.random() - 0.5) * 5,
            (math.random() - 0.5) * 5,
            (math.random() - 0.5) * 5
        }), DROP_FORCE)

        local drop = entity.spawn("base:drop", ppos)
        drop.rigidbody:set_vel(vec3.add(throw_force, vec3.add(pvel, DROP_INIT_VEL)))
        drop.transform:set_rot(mat4.rotate(mat4.rotate(mat4.rotate({0, 1, 0}, math.random() * 360), 
            {1, 0, 0}, math.random() * 360), {0, 0, 1}, math.random() * 360))
        end
    end)
end
