-- TODO: delete this file after caves complete implementation

function generate_heightmap(x, y, w, h, seed, s)
    local map = Heightmap(w, h)
    map:add(0.25)
    return map
end


function place_structures(x, z, w, d, seed, hmap, chunk_height)
    local placements = {}
    do
        local sy = math.random() * (chunk_height / 4)
        local ey = math.random() * (chunk_height / 4)
        local sx = x + math.random() * 20 - 10
        local ex = x + math.random() * 20 - 10
        local sz = z + math.random() * 20 - 10
        local ez = z + math.random() * 20 - 10
        table.insert(placements, 
            {":line", 0, {sx - 10, sy, sz - 10}, {ex + 10, ey, ez + 10}, 2})
    end
    return placements
end
