function on_open()
    refresh()
end

add_packs = {}
rem_packs = {}

function apply()
    core.reconfig_packs(add_packs, rem_packs)
    menu:back()
end

function refresh_changes()
    document.apply_btn.enabled = (#add_packs>0) or (#rem_packs>0)
end

function move_pack(id)
    -- cancel pack addition
    if table.has(add_packs, id) then
        document["pack_"..id]:move_into(document.packs_add)
        table.remove_value(add_packs, id)
    -- cancel pack removal
    elseif table.has(rem_packs, id) then
        document["pack_"..id]:move_into(document.packs_cur)
        table.remove_value(rem_packs, id)
    -- add pack
    elseif table.has(packs_installed, id) then
        document["pack_"..id]:move_into(document.packs_add)
        table.insert(rem_packs, id)
    -- remove pack
    else
        document["pack_"..id]:move_into(document.packs_cur)
        table.insert(add_packs, id)
    end
    refresh_changes()
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

function check_dependencies(packinfo)
    if packinfo.dependencies == nil then
        return
    end
    for i,dep in ipairs(packinfo.dependencies) do
        local depid = dep:sub(2,-1)
        if dep:sub(1,1) == '!' then 
            if not table.has(packs_all, depid) then
                packinfo.description = ""
                return string.format(
                    "%s (%s)", gui.str("error.dependency-not-found"), depid
                )
            end
            if table.has(packs_installed, packinfo.id) then
                table.insert(required, depid)
            end
        end
    end
    return
end

function refresh()
    packs_installed = pack.get_installed()
    packs_available = pack.get_available()
    base_packs = pack.get_base_packs()
    packs_all = {unpack(packs_installed)}
    required = {}
    for i,k in ipairs(packs_available) do
        table.insert(packs_all, k)
    end

    local packs_cur = document.packs_cur
    local packs_add = document.packs_add
    packs_cur:clear()
    packs_add:clear()

    for i,id in ipairs(packs_installed) do
        local packinfo = pack.get_info(id)
        packinfo.index = i
        callback = not table.has(base_packs, id) and string.format('move_pack("%s")', id) or nil
        packinfo.error = check_dependencies(packinfo)
        place_pack(packs_cur, packinfo, callback)
    end

    for i,id in ipairs(packs_available) do
        local packinfo = pack.get_info(id)
        packinfo.index = i
        callback = string.format('move_pack("%s")', id)
        packinfo.error = check_dependencies(packinfo)
        place_pack(packs_add, packinfo, callback)
    end

    for i,id in ipairs(packs_installed) do
        if table.has(required, id) then
            document["pack_"..id].enabled = false
        end
    end

    apply_movements(packs_cur, packs_add)
    refresh_changes()
end

function apply_movements(packs_cur, packs_add)
    for i,id in ipairs(packs_installed) do
        if table.has(rem_packs, id) then
            document["pack_"..id]:move_into(packs_add)
        end
    end
    for i,id in ipairs(packs_available) do
        if table.has(add_packs, id) then
            document["pack_"..id]:move_into(packs_cur)
        end
    end
end
