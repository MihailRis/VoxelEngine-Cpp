local tsf = entity.transform
local body = entity.rigidbody
local rig = entity.skeleton

local itemid = 0
local itemIndex = rig:index("item")

local function refresh_model(id)
    itemid = id
    rig:set_model(itemIndex, item.model_name(itemid))
    rig:set_matrix(itemIndex, mat4.rotate({0, 1, 0}, -80))
end

function on_render()
    local invid, slotid = player.get_inventory()
    local id, _ = inventory.get(invid, slotid)
    if id ~= itemid then
        refresh_model(id)
    end
end

