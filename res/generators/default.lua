biome_parameters = 0

biomes = {flat = {
    parameters = {},
    layers = {
        {height=-1, block="core:obstacle"},
    },
    sea_layers = {
        {height=-1, block="core:obstacle"}
    }
}}

function generate_heightmap(x, y, w, h, seed)
    return Heightmap(w, h)
end
