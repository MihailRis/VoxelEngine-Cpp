function add_pack(packid, packinfo)
    document.packs_panel:add(gui.template("pack", {
        id=packid,
        title=packinfo.title,
        description=packinfo.description,
        icon="gui/no_icon",
        creator=packinfo.creator,
        remover='0'
    }))
end

function on_open()
    local packs = pack.get_installed()
    for i,id in ipairs(packs) do
        add_pack(id, pack.get_info(id))
    end
end
