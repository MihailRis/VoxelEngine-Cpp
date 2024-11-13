local packs_installed = {}

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
end

function refresh_search()
    local search_text = document.search_textbox.text:lower()
    local visible = 0
    local interval = 4
    local step = -1

    for i, v in ipairs(packs_installed) do
        local id = v[1]
        local title = v[2]
        local content = document["pack_" .. id]
        local pos = content.pos
        local size = content.size

        if title:lower():find(search_text) or search_text == '' then
            content.enabled = true
            content.pos = {pos[1], visible * (size[2] + interval) - step}
            visible = visible + 1
        else
            content.enabled = false
            content.pos = {pos[1], (visible + #packs_installed - i) * (size[2] + interval) - step}
        end
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
    local packs_available = pack.get_available()
    packs_installed = pack.get_installed()

    for i,k in ipairs(packs_available) do
        table.insert(packs_installed, k)
    end

    local contents = document.contents
    contents:clear()

    for i,id in ipairs(packs_installed) do
        local packinfo = pack.get_info(id)

        packinfo.id = id
        packs_installed[i] = {packinfo.id, packinfo.title}
        local callback = string.format('open_pack("%s")', id)
        place_pack(contents, packinfo, callback)
    end
end
