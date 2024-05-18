function create_setting(id, name, step, postfix)
    local info = core.get_setting_info(id)
    postfix = postfix or ""
    document.root:add(gui.template("track_setting", {
        id=id,
        name=gui.str(name, "settings"),
        value=core.get_setting(id),
        min=info.min,
        max=info.max,
        step=step,
        postfix=postfix
    }))
    update_setting(core.get_setting(id), id, name, postfix)
end

function update_setting(x, id, name, postfix)
    core.set_setting(id, x)
    -- updating label
    document[id..".L"].text = string.format(
        "%s: %s%s", 
        gui.str(name, "settings"), 
        core.str_setting(id), 
        postfix
    )
end

function on_open()
    create_setting("audio.volume-master", "Master Volume", 0.01)
    create_setting("audio.volume-regular", "Regular Sounds", 0.01)
    create_setting("audio.volume-ui", "UI Sounds", 0.01)
    create_setting("audio.volume-ambient", "Ambient", 0.01)
    create_setting("audio.volume-music", "Music", 0.01)
end
