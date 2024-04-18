function add_pack(packid, packinfo)
    local callback = string.format('core.add_packs({%q})', packid)
    packinfo.id = packid
    packinfo.callback = callback
    packinfo.icon = "gui/no_icon"
    document.packs_panel:add(gui.template("pack", packinfo))
end

function on_open()
    local packs = pack.get_available()
    for i,id in ipairs(packs) do
        add_pack(id, pack.get_info(id))
    end
end
