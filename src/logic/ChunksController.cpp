#include "ChunksController.hpp"

#include "../content/Content.hpp"
#include "../voxels/Block.hpp"
#include "../voxels/Chunk.hpp"
#include "../voxels/Chunks.hpp"
#include "../voxels/ChunksStorage.hpp"
#include "../voxels/WorldGenerator.hpp"
#include "../world/WorldGenerators.hpp"
#include "../graphics/core/Mesh.hpp"
#include "../lighting/Lighting.hpp"
#include "../files/WorldFiles.hpp"
#include "../world/Level.hpp"
#include "../world/World.hpp"
#include "../maths/voxmaths.hpp"
#include "../util/timeutil.hpp"

#include <limits.h>
#include <memory>
#include <iostream>

const uint MAX_WORK_PER_FRAME = 128;
const uint MIN_SURROUNDING = 9;

ChunksController::ChunksController(Level* level, uint padding) 
  : level(level), 
    chunks(level->chunks.get()), 
    lighting(level->lighting.get()), 
    padding(padding), 
    generator(WorldGenerators::createGenerator(level->getWorld()->getGenerator(), level->content)) {
}

ChunksController::~ChunksController() = default;

void ChunksController::update(int64_t maxDuration) {
    int64_t mcstotal = 0;

    for (uint i = 0; i < MAX_WORK_PER_FRAME; i++) {
        timeutil::Timer timer;
        if (loadVisible()) {
            int64_t mcs = timer.stop();
            if (mcstotal + mcs < maxDuration * 1000) {
                mcstotal += mcs;
                continue;
            }
        }
        break;
    }
}

bool ChunksController::loadVisible(){
    const int w = chunks->w;
    const int d = chunks->d;

    int nearX = 0;
    int nearZ = 0;
    int minDistance = ((w-padding*2)/2)*((w-padding*2)/2);
    for (uint z = padding; z < d-padding; z++){
        for (uint x = padding; x < w-padding; x++){
            int index = z * w + x;
            auto& chunk = chunks->chunks[index];
            if (chunk != nullptr){
                if (chunk->flags.loaded && !chunk->flags.lighted) {
                    if (buildLights(chunk)) {
                        return true;
                    }
                }
                continue;
            }
            int lx = x - w / 2;
            int lz = z - d / 2;
            int distance = (lx * lx + lz * lz);
            if (distance < minDistance){
                minDistance = distance;
                nearX = x;
                nearZ = z;
            }
        }
    }

    const auto& chunk = chunks->chunks[nearZ * w + nearX];
    if (chunk != nullptr) {
        return false;
    }

    const int ox = chunks->ox;
    const int oz = chunks->oz;
    createChunk(nearX+ox, nearZ+oz);
    return true;
}

bool ChunksController::buildLights(const std::shared_ptr<Chunk>& chunk) {
    int surrounding = 0;
    for (int oz = -1; oz <= 1; oz++){
        for (int ox = -1; ox <= 1; ox++){
            if (chunks->getChunk(chunk->x+ox, chunk->z+oz))
                surrounding++;
        }
    }
    if (surrounding == MIN_SURROUNDING) {
        bool lightsCache = chunk->flags.loadedLights;
        if (!lightsCache) {
            lighting->buildSkyLight(chunk->x, chunk->z);
        }
        lighting->onChunkLoaded(chunk->x, chunk->z, !lightsCache);
        chunk->flags.lighted = true;
        return true;
    }
    return false;
}

void ChunksController::createChunk(int x, int z) {
    auto chunk = level->chunksStorage->create(x, z);
    chunks->putChunk(chunk);

    if (!chunk->flags.loaded) {
        generator->generate(
            chunk->voxels, x, z, 
            level->getWorld()->getSeed()
        );
        chunk->flags.unsaved = true;
    }
    chunk->updateHeights();

    if (!chunk->flags.loadedLights) {
        Lighting::prebuildSkyLight(
            chunk.get(), level->content->getIndices()
        );
    }
    chunk->flags.loaded = true;
    chunk->flags.ready = true;
}
