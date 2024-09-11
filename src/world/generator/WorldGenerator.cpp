#include "WorldGenerator.hpp"

#include <cstring>
#include <iostream>

#include "maths/util.hpp"
#include "content/Content.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunk.hpp"
#include "world/generator/GeneratorDef.hpp"
#include "util/timeutil.hpp"
#include "debug/Logger.hpp"

static debug::Logger logger("world-generator");

static inline constexpr uint MAX_PARAMETERS = 16;
static inline constexpr uint MAX_CHUNK_PROTOTYPE_LEVELS = 8;

WorldGenerator::WorldGenerator(
    const GeneratorDef& def, const Content* content, uint64_t seed
)
    : def(def), 
      content(content), 
      seed(seed), 
      surroundMap(0, MAX_CHUNK_PROTOTYPE_LEVELS) 
{
    surroundMap.setOutCallback([this](int const x, int const z, int8_t) {
        const auto& found = prototypes.find({x, z});
        if (found == prototypes.end()) {
            logger.warning() << "unable to remove non-existing chunk prototype";
            return;
        }
        prototypes.erase({x, z});
    });
    surroundMap.setLevelCallback(1, [this](int const x, int const z) {
        if (prototypes.find({x, z}) != prototypes.end()) {
            return;
        }
        prototypes[{x, z}] = generatePrototype(x, z);
    });
}

static inline void generate_pole(
    const BlocksLayers& layers,
    int top, int bottom,
    int seaLevel,
    voxel* voxels,
    int x, int z
) {
    uint y = top;
    uint layerExtension = 0;
    for (const auto& layer : layers.layers) {
        // skip layer if can't be generated under sea level
        if (y < seaLevel && !layer.belowSeaLevel) {
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
        layerHeight = std::min(static_cast<uint>(layerHeight), y+1);

        for (uint i = 0; i < layerHeight; i++, y--) {
            voxels[vox_index(x, y, z)].id = layer.rt.id;
        }
        layerExtension = 0;
    }
}

static inline const Biome* choose_biome(
    const std::vector<Biome>& biomes,
    const std::vector<std::shared_ptr<Heightmap>>& maps,
    uint x, uint z
) {
    uint paramsCount = maps.size();
    float params[MAX_PARAMETERS];
    for (uint i = 0; i < paramsCount; i++) {
        params[i] = maps[i]->getUnchecked(x, z);
    }
    const Biome* chosenBiome = nullptr;
    float chosenScore = std::numeric_limits<float>::infinity();
    for (const auto& biome : biomes) {
        float score = 0.0f;
        for (uint i = 0; i < paramsCount; i++) {
            score += glm::abs((params[i] - biome.parameters[i].value) / 
                              biome.parameters[i].weight);
        }
        if (score < chosenScore) {
            chosenScore = score;
            chosenBiome = &biome;
        }
    }
    return chosenBiome;
}

std::unique_ptr<ChunkPrototype> WorldGenerator::generatePrototype(
    int chunkX, int chunkZ
) {
    auto biomeParams = def.script->generateParameterMaps(
        {chunkX * CHUNK_W, chunkZ * CHUNK_D}, {CHUNK_W, CHUNK_D}, seed);
    const auto& biomes = def.script->getBiomes();

    std::vector<const Biome*> chunkBiomes(CHUNK_W*CHUNK_D);
    for (uint z = 0; z < CHUNK_D; z++) {
        for (uint x = 0; x < CHUNK_W; x++) {
            chunkBiomes[z * CHUNK_W + x] = choose_biome(biomes, biomeParams, x, z);
        }
    }
    return std::make_unique<ChunkPrototype>(
        ChunkPrototypeLevel::BIOMES,
        nullptr, 
        std::move(chunkBiomes));
}

void WorldGenerator::generateHeightmap(
    ChunkPrototype* prototype, int chunkX, int chunkZ
) {
    prototype->heightmap = def.script->generateHeightmap(
        {chunkX * CHUNK_W, chunkZ * CHUNK_D}, {CHUNK_W, CHUNK_D}, seed);
    prototype->level = ChunkPrototypeLevel::HEIGHTMAP;
}

void WorldGenerator::update(int centerX, int centerY, int loadDistance) {
    surroundMap.setCenter(centerX, centerY);
    surroundMap.resize(loadDistance);
    surroundMap.setCenter(centerX, centerY);
}

void WorldGenerator::generate(voxel* voxels, int chunkX, int chunkZ) {
    //timeutil::ScopeLogTimer log(555);
    surroundMap.completeAt(chunkX, chunkZ);

    const auto& found = prototypes.find({chunkX, chunkZ});
    if (found == prototypes.end()) {
        throw std::runtime_error("no prototype found");
    }

    auto prototype = found->second.get();
    generateHeightmap(prototype, chunkX, chunkZ);

    const auto values = prototype->heightmap->getValues();

    uint seaLevel = def.script->getSeaLevel();

    std::memset(voxels, 0, sizeof(voxel) * CHUNK_VOL);

    PseudoRandom plantsRand;
    plantsRand.setSeed(chunkX, chunkZ);

    for (uint z = 0; z < CHUNK_D; z++) {
        for (uint x = 0; x < CHUNK_W; x++) {
            const Biome* biome = prototype->biomes[z * CHUNK_W + x];

            int height = values[z * CHUNK_W + x] * CHUNK_H;
            height = std::max(0, height);

            const auto& groundLayers = biome->groundLayers;
            const auto& seaLayers = biome->seaLayers;

            generate_pole(seaLayers, seaLevel, height, seaLevel, voxels, x, z);
            generate_pole(groundLayers, height, 0, seaLevel, voxels, x, z);
            
            if (height+1 > seaLevel) {
                float rand = (plantsRand.randU32() % RAND_MAX) / 
                              static_cast<float>(RAND_MAX);
                blockid_t plant = biome->plants.choose(rand);
                if (plant) {
                    voxels[vox_index(x, height+1, z)].id = plant;
                }
            }
        }
    }
}
