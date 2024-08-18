#include "WorldGenerator.hpp"

#include <cstring>
#include <iostream>

#include "content/Content.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/voxel.hpp"
#include "world/generator/GeneratorDef.hpp"

WorldGenerator::WorldGenerator(const GeneratorDef& def, const Content* content)
    : def(def), content(content) {
}

static inline void generate_pole(
    const BlocksLayers& layers,
    int height,
    int bottom,
    int seaLevel,
    voxel* voxels,
    int x,
    int z
) {
    uint y = height;
    uint layerExtension = 0;
    for (const auto& layer : layers.layers) {
        // skip layer if can't be generated under sea level
        if (y < seaLevel && !layer.below_sea_level) {
            layerExtension = std::max(0, layer.height);
            continue;
        }

        int layerHeight = layer.height;
        if (layerHeight == -1) {
            // resizeable layer
            layerHeight = y - layers.lastLayersHeight - bottom + 1;
        } else {
            layerHeight += layerExtension;
        }
        layerHeight = std::min(static_cast<uint>(layerHeight), y);

        for (uint i = 0; i < layerHeight; i++, y--) {
            voxels[vox_index(x, y, z)].id = layer.rt.id;
        }
        layerExtension = 0;
    }
}

#include "util/timeutil.hpp"
void WorldGenerator::generate(
    voxel* voxels, int chunkX, int chunkZ, uint64_t seed
) {
    timeutil::ScopeLogTimer log(555);
    auto heightmap = def.script->generateHeightmap(
        {chunkX * CHUNK_W, chunkZ * CHUNK_D}, {CHUNK_W, CHUNK_D}, seed
    );
    auto values = heightmap->getValues();
    const auto& groundLayers = def.script->getGroundLayers();
    const auto& seaLayers = def.script->getSeaLayers();

    uint seaLevel = def.script->getSeaLevel();

    std::memset(voxels, 0, sizeof(voxel) * CHUNK_VOL);

    for (uint z = 0; z < CHUNK_D; z++) {
        for (uint x = 0; x < CHUNK_W; x++) {
            int height = values[z * CHUNK_W + x] * CHUNK_H;
            height = std::max(0, height);

            generate_pole(seaLayers, seaLevel, height, seaLevel, voxels, x, z);
            generate_pole(groundLayers, height, 0, seaLevel, voxels, x, z);
        }
    }
}
