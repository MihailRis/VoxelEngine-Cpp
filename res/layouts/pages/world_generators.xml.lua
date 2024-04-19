settings = session.get_entry('new_world')

function on_open()
    local names = core.get_generators()
    table.sort(names)

    local panel = document.root
    for _,k in ipairs(names) do
        panel:add(gui.template("generator", {
            callback=string.format("settings.generator=%q menu:back()", k),
            id=k,
            name=settings.generator_name(k)
        }))
    end
    panel:add("<button padding='10' onclick='menu:back()'>@Back</button>")
end
