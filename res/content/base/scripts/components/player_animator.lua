local tsf = entity.transform
local body = entity.rigidbody
local rig = entity.skeleton

local itemid = 0
local itemIndex = rig:index("item")

local function refresh_model(id)
    itemid = id
    rig:set_model(itemIndex, item.name(itemid)..".model")
end

function on_render()
    local invid, slotid = player.get_inventory()
    local id, _ = inventory.get(invid, slotid)
    if id ~= itemid then
        refresh_model(id)
    end
end

