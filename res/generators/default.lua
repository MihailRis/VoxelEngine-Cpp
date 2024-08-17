sea_level = 64

sea_layers = {
    {block="base:water", height=-1},
}

layers = {
    {block="base:grass_block", height=1, below_sea_level=false},
    {block="base:dirt", height=5, below_sea_level=false},
    {block="base:stone", height=-1},
    {block="base:bazalt", height=1},
}

local function _generate_heightmap(x, y, w, h, seed, s)
    local umap = Heightmap(w, h)
    local vmap = Heightmap(w, h)
    umap.noiseSeed = seed
    vmap.noiseSeed = seed
    vmap:noise({x+521, y+70}, 0.1*s, 3, 25.8)
    vmap:noise({x+95, y+246}, 0.15*s, 3, 25.8)

    local map = Heightmap(w, h)
    map.noiseSeed = seed
    map:noise({x, y}, 0.8*s, 4, 0.04)
    map:cellnoise({x, y}, 0.1*s, 3, 0.7, umap, vmap)
    map:mul(0.5)
    map:add(0.5)

    local rivermap = Heightmap(w, h)
    rivermap.noiseSeed = seed
    rivermap:noise({x+21, y+12}, 0.1*s, 4)
    rivermap:abs()
    rivermap:mul(2.0)
    rivermap:pow(0.3)
    rivermap:max(0.6)
    map:add(0.4)
    map:mul(rivermap)
    map:add(-0.15)

    return map
end

function generate_heightmap(x, y, w, h, seed)
    -- blocks per dot
    -- 8 - linear interpolation is visible, but not so much (Minecraft)
    -- 4 - high quality, but slower
    -- 2 - you really don't need it
    -- 1 - please have mercy on your CPU
    local bpd = 8
    local map = _generate_heightmap(
        math.floor(x/bpd), math.floor(y/bpd), 
        math.floor(w/bpd)+1, math.floor(h/bpd)+1, seed, bpd)
    map:resize(w+bpd, h+bpd, 'linear')
    map:crop(0, 0, w, h)
    return map
end
