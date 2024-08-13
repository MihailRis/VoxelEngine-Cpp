-- use for engine development tests
-- must be empty in release
-- must not be modified by content-packs

local W = 16
local H = 16

for t=1,1 do
    local tm = time.uptime()

    local umap = Heightmap(W, H)
    local vmap = Heightmap(W, H)
    umap:noise({521, 73}, 0.2, 1, 11.8)
    umap:noise({51, 73}, 0.2, 1, 11.8)
    umap:noise({521, 73}, 0.4, 6, 5.8)
    vmap:noise({95, 246}, 0.6, 6, 5.8)

    local bmap = Heightmap(W, H)
    bmap:noise({3, 6}, 0.1, 1, 3)
    local map = Heightmap(W, H)


    map:noise({0, 0}, 0.06, 4, 0.2, umap, vmap)
    map:noise({0, 0}, 0.25, 6, 0.5, umap, vmap)
    map:mul(bmap)
    map:mul(0.7)

    local rivermap = Heightmap(W, H)
    rivermap:noise({21, 12}, 0.05, 3)
    rivermap:abs()
    rivermap:min(0.02)
    rivermap:mul(50.0)
    rivermap:pow(0.4)
    map:add(1.7)
    map:mul(rivermap)
    map:add(-1.0)
    map:mul(0.5)

    local overmap = Heightmap(W, H)
    overmap:noise({1, 5122}, 0.02, 2, 0.2)
    overmap:abs()
    overmap:pow(0.5)
    overmap:mul(-1.0)
    map:add(overmap)

    print(math.floor((time.uptime() - tm) * 1000000).." mcs")
    map:dump("heightmap.png")
end
