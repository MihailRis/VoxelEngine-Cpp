function on_open()
    new_volume_control("audio.volume-master", "master")
    new_volume_control("audio.volume-regular", "regular")
    new_volume_control("audio.volume-ui", "ui")
    new_volume_control("audio.volume-ambient", "ambient")
    new_volume_control("audio.volume-music", "music")

    gui.reindex("core:pages/settings-audio")

    on_master_change()
    on_regular_change()
    on_ui_change()
    on_ambient_change()
    on_music_change()
end


function new_volume_control(setting, id, name)
    -- value text label
    document.tracks_panel:add("<label id='l_"..id.."'>-</label>")
    -- value track-bar
    document.tracks_panel:add(string.format(
        "<trackbar id='t_%s' min='0' max='1' value='1' step='0.01' track-width='5' "..
        " consumer='on_%s_change'/>"
    , id, id))
end

function on_volume_change(setting, id, name, val)
    if val ~= nil then
        core.set_setting(setting, val)
    else
        document["t_"..id].value = core.get_setting(setting, val)
    end
    document["l_"..id].text = (
        gui.str(name, "settings")..": "..
        core.str_setting(setting)
    )
end

function on_master_change(val)
    on_volume_change("audio.volume-master", "master", "Master Volume", val)
end

function on_regular_change(val)
    on_volume_change("audio.volume-regular", "regular", "Regular Sounds", val)
end

function on_ui_change(val)
    on_volume_change("audio.volume-ui", "ui", "UI Sounds", val)
end

function on_ambient_change(val)
    on_volume_change("audio.volume-ambient", "ambient", "Ambient", val)
end

function on_music_change(val)
    on_volume_change("audio.volume-music", "music", "Music", val)
end
