local pack_title
function on_open(params)
    if params then
        mode = params.mode
    end
    document.next_button.text = ">";
    document.previous_button.text = "<";
    --refresh()
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

function download() 
    --print(pack_title)
    pack.voxelworld_download_pack(pack_title)
end

function next_page() 
    page = pack.voxelworld_get_page()
    pack.voxelworld_set_page(page+1)
    document.page_value.text = page+1
    refresh()
end

function previous_page() 
    page = pack.voxelworld_get_page()
    if page > 1 then
        pack.voxelworld_set_page(page-1)
        document.page_value.text = page-1
        refresh()
    end
end
function open_pack(id)
    local packinfo = pack.voxelworld_get_mod(id)
    
    if packinfo['dependencies'] == nil then document.dependencies.text = 'None' else document.dependencies.text = table.tostring(packinfo['dependencies']) end
    if packinfo['creator'] == '' then document.author.text = 'None' else document.author.text = packinfo['creator'] end
    if packinfo['version'] == nil then document.version.text = 'None' else document.version.text = packinfo['version'] end
    if packinfo['description'] == nil then document.description.text = 'None' else document.description.text = packinfo['description'] end
    pack_title = id
    document.pack_title_value.text = id
end

function refresh()
    local packs_installed = pack.voxelworld_get_mods()


    local contents = document.contents
    contents:clear()

    for i,id in ipairs(packs_installed) do
        local packinfo = pack.voxelworld_get_mod_from_mods(id)

        packinfo.index = i
        local callback = string.format('open_pack("%s")', id)
        place_pack(contents, packinfo, callback)
    end
end
