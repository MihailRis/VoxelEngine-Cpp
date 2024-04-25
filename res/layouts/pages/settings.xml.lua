function create_setting(id, name, step, postfix)
    local info = core.get_setting_info(id)
    if postfix == nil then
        postfix = ""
    end
    document.settings_panel:add(gui.template("track_setting", {
        id=id,
        name=gui.str(name, "settings"),
        value=core.get_setting(id),
        min=info.min,
        max=info.max,
        step=step,
        track_width=12,
        postfix=postfix
    }))
end

function update_setting(x, id, name, postfix)
    core.set_setting(id, x)
    -- updating label
    document[id..".L"].text = string.format(
        "%s: %s%s", name, core.str_setting(id), postfix
    )
end

function create_checkbox(id, name)
    document.settings_panel:add(string.format(
        "<checkbox consumer='function(x) core.set_setting(\"%s\", x) end' checked='%s'>%s</checkbox>", 
        id, core.str_setting(id), gui.str(name, "settings")
    ))
end

function on_open()
    create_setting("chunks.load-distance", "Load Distance", 1)
    create_setting("chunks.load-speed", "Load Speed", 1)
    create_setting("graphics.fog-curve", "Fog Curve", 0.1)
    create_setting("graphics.gamma", "Gamma", 0.05)
    create_setting("camera.fov", "FOV", 1, "°")
    create_checkbox("display.vsync", "V-Sync")
    create_checkbox("graphics.backlight", "Backlight")
    create_checkbox("camera.shaking", "Camera Shaking")
    document.langs_btn.text = string.format(
        "%s: %s", gui.str("Language", "settings"),
        gui.get_locales_info()[gui.get_locale()].name
    )
end
