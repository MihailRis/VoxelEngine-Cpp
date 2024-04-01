function on_open()
    on_master_change()
    on_regular_change()
    on_ui_change()
    on_ambient_change()
    on_music_change()
end

function on_volume_change(setting, label, name, val)
    if val ~= nil then
        core.set_setting(setting, val)
    end
    label.text = (
        gui.str(name, "settings")..": "..
        core.str_setting(setting)
    )
end

function on_master_change(val)
    on_volume_change("audio.volume-master", document.l_master, "Master Volume", val)
end

function on_regular_change(val)
    on_volume_change("audio.volume-regular", document.l_regular, "Regular Sounds", val)
end

function on_ui_change(val)
    on_volume_change("audio.volume-ui", document.l_ui, "UI Sounds", val)
end

function on_ambient_change(val)
    on_volume_change("audio.volume-ambient", document.l_ambient, "Ambient", val)
end

function on_music_change(val)
    on_volume_change("audio.volume-music", document.l_music, "Music", val)
end
