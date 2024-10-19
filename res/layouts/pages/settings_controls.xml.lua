
function refresh_search()
    local search_text = document.search_textbox.text
    local search_key = document.search_key_checkbox.checked

    local panel = document.bindings_panel
    local bindings = input.get_bindings()
    panel:clear()

    table.sort(bindings, function(a, b) return a > b end)
    if search_text ~= "" then
        for i,name in ipairs(bindings) do
            local _name = gui.str(name)
            if ((_name:lower():find(search_text:lower()) and not search_key) or
                (input.get_binding_text(name):lower():find(search_text:lower()) and search_key)) then
                panel:add(gui.template("binding", {
                    id=name, name=_name
                }))
            end
        end
    else
        for i,name in ipairs(bindings) do
            panel:add(gui.template("binding", {
                id=name, name=gui.str(name)
            }))
        end
    end
end

function refresh_sensitivity()
    document.sensitivity_label.text = string.format(
        "%s: %s", 
        gui.str("Mouse Sensitivity", "settings"),
        core.str_setting("camera.sensitivity")
    )
end

function change_sensitivity(val)
    core.set_setting("camera.sensitivity", val)
    refresh_sensitivity()
end

function on_open()
    document.sensitivity_track.value = core.get_setting("camera.sensitivity")
    refresh_sensitivity()

    document.search_panel:add(string.format(
        "<checkbox size='20,20' id='search_key_checkbox' consumer='function(x) refresh_search() end' tooltip='%s'>%s</checkbox>", 
        gui.str("controls.key.tooltip", "settings"), gui.str("Key", "settings")
    ))

    local panel = document.bindings_panel
    local bindings = input.get_bindings()
    table.sort(bindings, function(a, b) return a > b end)
    for i,name in ipairs(bindings) do
        panel:add(gui.template("binding", {
            id=name, name=gui.str(name)
        }))
    end
end
