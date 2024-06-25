function on_hud_open()
    input.add_callback("player.drop", function ()
        local pid = hud.get_player()
        local pvel = {player.get_vel(pid)}
        local eid = entity.test()
        entity.set_vel(eid, pvel)
    end)
end
