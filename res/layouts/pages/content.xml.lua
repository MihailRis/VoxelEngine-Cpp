function on_open(params)
    if params then
        mode = params.mode
    end
    refresh()
end

add_packs = {}
rem_packs = {}

packs_included = {}
packs_excluded = {}

packs_info = {}

local function include(id, is_include)
    if is_include then
        table.insert(packs_included, id)
        table.remove_value(packs_excluded, id)
    else
        table.insert(packs_excluded, id)
        table.remove_value(packs_included, id)
    end
end

function apply()
    core.reconfig_packs(add_packs, rem_packs)
    if mode ~= "world" then
        menu:back()
    end
end

function refresh_search()
    local search_text = document.search_textbox.text:lower()
    local interval = 2
    local step = -1

    for _, packs in ipairs({packs_excluded, packs_included}) do
        local visible = 0

        for i, v in ipairs(packs) do
            local info = packs_info[v]

            local id = info[1]
            local title = info[2]

            local content = document["pack_" .. id]
            local pos = content.pos
            local size = content.size

            if title:lower():find(search_text) or search_text == '' then
                content.pos = {pos[1], visible * (size[2] + interval) - step}
                visible = visible + 1
            else
                content.pos = {pos[1], (visible + #packs - i) * (size[2] + interval) - step}
            end
        end
    end
end

function refresh_changes()
    document.apply_btn.enabled = (#add_packs>0) or (#rem_packs>0)
    refresh_search()
end

function move_pack(id)
    -- cancel pack addition
    if table.has(add_packs, id) then
        document["pack_"..id]:moveInto(document.packs_add)
        table.remove_value(add_packs, id)
        include(id, false)
    -- cancel pack removal
    elseif table.has(rem_packs, id) then
        document["pack_"..id]:moveInto(document.packs_cur)
        table.remove_value(rem_packs, id)
        include(id, true)
    -- add pack
    elseif table.has(packs_installed, id) then
        document["pack_"..id]:moveInto(document.packs_add)
        table.insert(rem_packs, id)
        include(id, false)
    -- remove pack
    else
        document["pack_"..id]:moveInto(document.packs_cur)
        table.insert(add_packs, id)
        include(id, true)
    end
    refresh_changes()
end

function move_left()
    for _, id in pairs(table.copy(packs_excluded)) do
        if not document["pack_"..id].enabled then goto continue end

        include(id, true)
        table.insert(add_packs, id)
        table.remove_value(rem_packs, id)
        document["pack_"..id]:moveInto(document.packs_cur)

        ::continue::
    end

    refresh_changes()
end

function move_right()
    for _, id in pairs(table.copy(packs_included)) do
        if not document["pack_"..id].enabled then goto continue end

        include(id, false)

        if table.has(packs_installed, id) then
            table.insert(rem_packs, id)
        end

        table.remove_value(add_packs, id)
        document["pack_"..id]:moveInto(document.packs_add)

        ::continue::
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

    -- lock pack that contains used generator
    if world.is_open() then
        local genpack, genname = parse_path(world.get_generator())
        if genpack ~= "core" then
            table.insert(base_packs, genpack)
        end
    end

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

    for _,id in ipairs(base_packs) do
        local packinfo = pack.get_info(id)
        packs_info[id] = {packinfo.id, packinfo.title}
    end

    for _,id in ipairs(packs_all) do
        local packinfo = pack.get_info(id)
        packs_info[id] = {packinfo.id, packinfo.title}
    end

    for i,id in ipairs(packs_installed) do
        if table.has(required, id) then
            document["pack_"..id].enabled = false
        end
    end

    if #packs_excluded == 0 then packs_excluded = table.copy(packs_available) end
    if #packs_included == 0 then packs_included = table.copy(packs_installed) end

    apply_movements(packs_cur, packs_add)
    refresh_changes()
end

function apply_movements(packs_cur, packs_add)
    for i,id in ipairs(packs_installed) do
        if table.has(rem_packs, id) then
            document["pack_"..id]:moveInto(packs_add)
        end
    end
    for i,id in ipairs(packs_available) do
        if table.has(add_packs, id) then
            document["pack_"..id]:moveInto(packs_cur)
        end
    end
end
