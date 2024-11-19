function on_open()
    local worlds = world.get_list()
    for _, info in ipairs(worlds) do
        document.worlds:add(gui.template("world", info))
    end
end
