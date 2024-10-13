settings = session.get_entry('new_world')

function on_open()
    local names = generation.get_generators()
    table.sort(names)

    local panel = document.root
    for k, caption in pairs(names) do
        panel:add(gui.template("generator", {
            callback=string.format("settings.generator=%q menu:back()", k),
            id=k,
            name=settings.generator_name(caption)
        }))
    end
    panel:add("<button onclick='menu:back()'>@Back</button>")
end
