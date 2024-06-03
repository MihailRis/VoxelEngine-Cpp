function on_open()
    local locales = gui.get_locales_info()
    local invlocales = {}
    local names = {}
    for k, v in pairs(locales) do
        table.insert(names, v.name)
        invlocales[v.name] = k
    end
    table.sort(names)
    
    local panel = document.root
    for _,name in ipairs(names) do
        panel:add(gui.template(
            "language",  {id=invlocales[name], name=name}
        ))
    end
    panel:add("<button onclick='menu:back()'>@Back</button>")
end
