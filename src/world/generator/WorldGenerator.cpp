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
void WorldGenerator::generate(voxel* voxels, int chunkX, int chunkZ, int seed) {
    auto heightmap = def.script->generateHeightmap(
        {chunkX*CHUNK_W, chunkZ*CHUNK_D}, {CHUNK_W, CHUNK_D}
    );
    timeutil::ScopeLogTimer log(555);
    auto values = heightmap->getValues();
    const auto& layers = def.script->getLayers();
    uint lastLayersHeight = def.script->getLastLayersHeight();
    auto baseWater = content->blocks.require("base:water").rt.id;

    std::memset(voxels, 0, sizeof(voxel) * CHUNK_VOL);

    for (uint z = 0; z < CHUNK_D; z++) {
        for (uint x = 0; x < CHUNK_W; x++) {
            int height = values[z * CHUNK_W + x] * 255 + 10;
            for (uint y = height+1; y < 64; y++) {
                voxels[vox_index(x, y, z)].id = baseWater;
            }

            uint y = height;
            for (const auto& layer : layers) {
                uint layerHeight = layer.height;
                if (layerHeight == -1) {
                    layerHeight = y - lastLayersHeight + 1;
                }
                for (uint i = 0; i < layerHeight; i++, y--) {
                    voxels[vox_index(x, y, z)].id = layer.rt.id;
                }
            }
        }
    }
}
