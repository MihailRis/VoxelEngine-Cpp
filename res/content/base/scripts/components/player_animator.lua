local tsf = entity.transform
local body = entity.rigidbody
local rig = entity.skeleton

local itemid = 0
local headIndex = rig:index("head")
local itemIndex = rig:index("item")
local bodyIndex = rig:index("body")

local function refresh_model(id)
    itemid = id
    rig:set_model(itemIndex, item.model_name(itemid))
    rig:set_matrix(itemIndex, mat4.rotate({0, 1, 0}, -80))
end

function on_render()
    local pid = entity:get_player()
    if pid == -1 then
        return
    end
    
    local rx, ry, rz = player.get_rot(pid, pid ~= hud.get_player())
    rig:set_matrix(headIndex, mat4.rotate({1, 0, 0}, ry))
    rig:set_matrix(bodyIndex, mat4.rotate({0, 1, 0}, rx))

    local invid, slotid = player.get_inventory(pid)
    local id, _ = inventory.get(invid, slotid)
    if id ~= itemid then
        refresh_model(id)
    end
end

