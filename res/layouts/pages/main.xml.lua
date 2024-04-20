function on_open()
    local worlds = core.get_worlds_list()
    for _, name in ipairs(worlds) do
        document.worlds:add(gui.template("world", {name=name}))
    end
end
