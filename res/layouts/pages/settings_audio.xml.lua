function on_open()
    new_volume_control("audio.volume-master", "master", "Master Volume")
    new_volume_control("audio.volume-regular", "regular", "Regular Sounds")
    new_volume_control("audio.volume-ui", "ui", "UI Sounds")
    new_volume_control("audio.volume-ambient", "ambient", "Ambient")
    new_volume_control("audio.volume-music", "music", "Music")
end

function new_volume_control(setting, id, name)
    -- value text label
    document.tracks_panel:add("<label id='l_"..id.."'>-</label>")
    -- value track-bar
    document.tracks_panel:add(string.format(
        "<trackbar id='t_%s' min='0' max='1' value='%s' step='0.01' track-width='5' "..
        " consumer='function(x) on_volume_change(%q, %q, %q, x) end'/>"
    , id, core.get_setting(setting), setting, id, name))
    refresh_label(setting, id, name)
end

function refresh_label(setting, id, name)
    document["l_"..id].text = (
        gui.str(name, "settings")..": "..
        core.str_setting(setting)
    )
end

function on_volume_change(setting, id, name, val)
    if val ~= nil then
        core.set_setting(setting, val)
    else
        document["t_"..id].value = core.get_setting(setting, val)
    end
    refresh_label(setting, id, name)
end
