function on_open()
    document.langs_btn.text = string.format(
        "%s: %s", gui.str("Language", "settings"),
        gui.get_locales_info()[core.get_setting("ui.language")].name
    )
    set_page("s_gfx", "settings_graphics")
end

function set_page(btn, page)
    document.s_aud.enabled = true
    document.s_dsp.enabled = true
    document.s_gfx.enabled = true
    document.s_ctl.enabled = true
    document.s_rst.enabled = true
    document[btn].enabled = false
    document.menu.page = page
end
