function on_open(params)
    refresh()
end

function place_pack(panel, packinfo, callback)
    if packinfo.error then
        callback = nil
    end
    if packinfo.has_indices then
        packinfo.id_verbose = packinfo.id.."*"
    else
        packinfo.id_verbose = packinfo.id
    end
    packinfo.callback = callback
    panel:add(gui.template("pack", packinfo))
    if not callback then
        document["pack_"..packinfo.id].enabled = false
    end
end

function open_pack(id)
    local packinfo = pack.get_info(id)

    if packinfo['dependencies'] == nil then document.dependencies.text = 'None' else document.dependencies.text = table.tostring(packinfo['dependencies']) end
    if packinfo['creator'] == '' then document.author.text = 'None' else document.author.text = packinfo['creator'] end
    if packinfo['version'] == nil then document.version.text = 'None' else document.version.text = packinfo['version'] end
    if packinfo['description'] == nil then document.description.text = 'None' else document.description.text = packinfo['description'] end
end

function refresh()
    local packs_installed = pack.get_installed()
    local packs_available = pack.get_available()

    for i,k in ipairs(packs_available) do
        table.insert(packs_installed, k)
    end

    local contents = document.contents
    contents:clear()

    for i,id in ipairs(packs_installed) do
        local packinfo = pack.get_info(id)

        packinfo.index = i
        local callback = string.format('open_pack("%s")', id)
        place_pack(contents, packinfo, callback)
    end
end
