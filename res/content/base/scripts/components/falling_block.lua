local tsf = entity.transform
local body = entity.rigidbody
local rig = entity.skeleton

ARGS = ARGS or {}
local blockid = ARGS.block
if SAVED_DATA.block then
    blockid = SAVED_DATA.block
else
    SAVED_DATA.block = blockid
end

do -- setup visuals
    local textures = block.get_textures(block.index(blockid))
    for i,t in ipairs(textures) do
        rig:set_texture("$"..tostring(i-1), "blocks:"..textures[i])
    end
end

function on_grounded()
    local pos = tsf:get_pos()
    local ix = math.floor(pos[1])
    local iy = math.floor(pos[2])
    local iz = math.floor(pos[3])
    if block.is_replaceable_at(ix, iy, iz) then
        block.set(ix, iy, iz, block.index(blockid))
    else
        local picking_item = block.get_picking_item(block.index(blockid))
        local drop = entities.spawn("base:drop", pos, {item={id=picking_item, count=1}})
        drop.rigidbody:set_vel(vec3.spherical_rand(5.0))
    end
    entity:despawn()
end
