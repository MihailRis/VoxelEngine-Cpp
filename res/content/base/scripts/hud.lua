function on_hud_open()
    input.add_callback("player.drop", function ()
        local pid = hud.get_player()
        local pvel = {player.get_vel(pid)}
        local eid = entity.test()
        entity.set_vel(eid, vec3.add(vec3.mul(player.get_dir(pid), {8, 8, 8}), vec3.add(pvel, {0, 3, 0})))
    end)
end
