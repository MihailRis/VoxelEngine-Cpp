function generate_heightmap(x, y, w, h)
    local umap = Heightmap(w, h)
    local vmap = Heightmap(w, h)
    umap:noise({x+521, y+73}, 0.05, 1, 20.8)
    umap:noise({x+51, y+75}, 0.05, 1, 21.8)
    vmap:noise({x+521, y+70}, 0.1, 3, 35.8)
    vmap:noise({x+95, y+246}, 0.15, 3, 35.8)

    local map = Heightmap(w, h)
    map:noise({x, y}, 0.02, 6, 0.2)
    map:noise({x, y}, 0.06, 7, 0.5, umap, vmap)
    map:mul(0.3)
    map:add(0.3)
    return map
end
