local item_models = require "core:item_models"

local tsf = entity.transform
local body = entity.rigidbody
local rig = entity.skeleton

local itemid = 0
local itemIndex = rig:index("item")

local function refresh_model(id)
    if id == 0 then
        rig:set_model(itemIndex, "")
    else
        itemid = id
        local scale = item_models.setup(itemid, rig, itemIndex)
        rig:set_matrix(itemIndex, mat4.scale(scale))
    end 
end

function on_render()
    local invid, slotid = player.get_inventory()
    local id, _ = inventory.get(invid, slotid)
    if id ~= itemid then
        refresh_model(id)
    end
end

