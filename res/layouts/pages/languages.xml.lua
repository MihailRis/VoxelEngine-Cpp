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
    for _,k in ipairs(names) do
        panel:add(string.format(
            "<button onclick=%q padding='10'>%s</button>",
            string.format("gui.set_locale(%q) menu:back()", invlocales[k]), k
        ))
    end
    panel:add("<button padding='10' onclick='menu:back()'>@Back</button>")
end
