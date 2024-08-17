#include "WorldGenerator.hpp"

#include <cstring>

#include "content/Content.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/voxel.hpp"
#include "world/generator/GeneratorDef.hpp"

WorldGenerator::WorldGenerator(
    const GeneratorDef& def,
    const Content* content
) : def(def), content(content) {
}

#include "util/timeutil.hpp"
void WorldGenerator::generate(
    voxel* voxels, int chunkX, int chunkZ, uint64_t seed
) {
    timeutil::ScopeLogTimer log(555);
    auto heightmap = def.script->generateHeightmap(
        {chunkX*CHUNK_W, chunkZ*CHUNK_D}, {CHUNK_W, CHUNK_D}, seed
    );
    auto values = heightmap->getValues();
    const auto& layers = def.script->getLayers();
    uint lastLayersHeight = def.script->getLastLayersHeight();
    uint seaLevel = def.script->getSeaLevel();
    auto baseWater = content->blocks.require("base:water").rt.id;

    std::memset(voxels, 0, sizeof(voxel) * CHUNK_VOL);

    for (uint z = 0; z < CHUNK_D; z++) {
        for (uint x = 0; x < CHUNK_W; x++) {
            // generate water
            int height = values[z * CHUNK_W + x] * CHUNK_H;
            for (uint y = height+1; y <= seaLevel; y++) {
                voxels[vox_index(x, y, z)].id = baseWater;
            }

            uint y = height;
            uint layerExtension = 0;
            for (const auto& layer : layers) {
                // skip layer if can't be generated under sea level
                if (y < seaLevel && !layer.below_sea_level) {
                    layerExtension = std::max(0, layer.height);
                    continue;
                }
                
                uint layerHeight = layer.height;
                if (layerHeight == -1) {
                    // resizeable layer
                    layerHeight = y - lastLayersHeight + 1;
                } else {
                    layerHeight += layerExtension;
                }
                for (uint i = 0; i < layerHeight; i++, y--) {
                    voxels[vox_index(x, y, z)].id = layer.rt.id;
                }
                layerExtension = 0;
            }
        }
    }
}
