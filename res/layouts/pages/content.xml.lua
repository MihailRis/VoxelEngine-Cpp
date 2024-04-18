function add_pack(packid, packinfo)
    local remover = ''
    if packid ~= "base" then
        remover = string.format('core.remove_packs({%q})', packid)
    end
    if packinfo.has_indices then
        packid = packid.."*"
    end
    packinfo.id = packid
    packinfo.remover = remover
    packinfo.icon = "gui/no_icon"
    document.packs_panel:add(gui.template("pack", packinfo))
end

function on_open()
    local packs = pack.get_installed()
    for i,id in ipairs(packs) do
        add_pack(id, pack.get_info(id))
    end
end
