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

    local panel = document.bindings_panel
    local bindings = core.get_bindings()
    for i,name in ipairs(bindings) do
        panel:add(gui.template("binding", {
            id=name, name=gui.str(name)
        }))
    end
end
