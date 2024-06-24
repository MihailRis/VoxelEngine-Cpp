function on_hud_open()
    input.add_callback("player.drop", function ()
        entity.test()
    end)
end
