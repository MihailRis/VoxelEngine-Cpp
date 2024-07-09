local tsf = entity.transform
local body = entity.rigidbody
local rig = entity.modeltree

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
        entities.spawn("base:drop", pos, {item={id=item.index(blockid..".item"), count=1}}) 
    end
    entity:despawn()
end
