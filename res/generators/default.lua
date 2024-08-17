sea_level = 64

layers = {
    {block="base:grass_block", height=1, below_sea_level=false},
    {block="base:dirt", height=5, below_sea_level=false},
    {block="base:stone", height=-1},
    {block="base:bazalt", height=1},
}

function generate_heightmap(x, y, w, h, seed)
    local umap = Heightmap(w, h)
    local vmap = Heightmap(w, h)
    umap.noiseSeed = seed
    vmap.noiseSeed = seed
    umap:noise({x+521, y+73}, 0.05, 1, 20.8)
    umap:noise({x+51, y+75}, 0.05, 1, 21.8)
    vmap:noise({x+521, y+70}, 0.1, 3, 35.8)
    vmap:noise({x+95, y+246}, 0.15, 3, 35.8)

    local map = Heightmap(w, h)
    map.noiseSeed = seed
    map:noise({x, y}, 0.02, 7, 0.2)
    map:noise({x, y}, 0.06, 8, 0.4, umap, vmap)
    map:mul(0.5)
    map:add(0.1)
    map:pow(2.0)

    local rivermap = Heightmap(w, h)
    rivermap.noiseSeed = seed
    rivermap:noise({x+21, y+12}, 0.1, 4)
    rivermap:abs()
    rivermap:mul(2.0)
    rivermap:pow(0.4)
    rivermap:max(0.6)
    map:add(0.4)
    map:mul(rivermap)
    map:add(-0.2)

    return map
end
