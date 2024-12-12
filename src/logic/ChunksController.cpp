#include "ChunksController.hpp"

#include <limits.h>

#include <iostream>
#include <memory>

#include "content/Content.hpp"
#include "files/WorldFiles.hpp"
#include "graphics/core/Mesh.hpp"
#include "lighting/Lighting.hpp"
#include "maths/voxmaths.hpp"
#include "util/timeutil.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/Chunks.hpp"
#include "voxels/ChunksStorage.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"
#include "world/generator/WorldGenerator.hpp"
#include "scripting/scripting.hpp"

const uint MAX_WORK_PER_FRAME = 128;
const uint MIN_SURROUNDING = 9;

ChunksController::ChunksController(Level& level, uint padding)
    : level(level),
      chunks(*level.chunks),
      lighting(*level.lighting),
      padding(padding),
      generator(std::make_unique<WorldGenerator>(
          level.content->generators.require(level.getWorld()->getGenerator()),
          level.content,
          level.getWorld()->getSeed()
      )) {}

ChunksController::~ChunksController() = default;

void ChunksController::update(
    int64_t maxDuration, int loadDistance, int centerX, int centerY
) {
    generator->update(centerX, centerY, loadDistance);

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

bool ChunksController::loadVisible() {
    int sizeX = chunks.getWidth();
    int sizeY = chunks.getHeight();

    int nearX = 0;
    int nearZ = 0;
    bool assigned = false;
    int minDistance = ((sizeX - padding * 2) / 2) * ((sizeY - padding * 2) / 2);
    for (uint z = padding; z < sizeY - padding; z++) {
        for (uint x = padding; x < sizeX - padding; x++) {
            int index = z * sizeX + x;
            auto& chunk = chunks.getChunks()[index];
            if (chunk != nullptr) {
                if (chunk->flags.loaded && !chunk->flags.lighted) {
                    if (buildLights(chunk)) {
                        return true;
                    }
                }
                continue;
            }
            int lx = x - sizeX / 2;
            int lz = z - sizeY / 2;
            int distance = (lx * lx + lz * lz);
            if (distance < minDistance) {
                minDistance = distance;
                nearX = x;
                nearZ = z;
                assigned = true;
            }
        }
    }

    const auto& chunk = chunks.getChunks()[nearZ * sizeX + nearX];
    if (chunk != nullptr || !assigned) {
        return false;
    }
    int offsetX = chunks.getOffsetX();
    int offsetY = chunks.getOffsetY();
    createChunk(nearX + offsetX, nearZ + offsetY);
    return true;
}

bool ChunksController::buildLights(const std::shared_ptr<Chunk>& chunk) {
    int surrounding = 0;
    for (int oz = -1; oz <= 1; oz++) {
        for (int ox = -1; ox <= 1; ox++) {
            if (chunks.getChunk(chunk->x + ox, chunk->z + oz)) surrounding++;
        }
    }
    if (surrounding == MIN_SURROUNDING) {
        bool lightsCache = chunk->flags.loadedLights;
        if (!lightsCache) {
            lighting.buildSkyLight(chunk->x, chunk->z);
        }
        lighting.onChunkLoaded(chunk->x, chunk->z, !lightsCache);
        chunk->flags.lighted = true;
        return true;
    }
    return false;
}

void ChunksController::createChunk(int x, int z) {
    auto chunk = level.chunksStorage->create(x, z);
    chunks.putChunk(chunk);
    auto& chunkFlags = chunk->flags;

    if (!chunkFlags.loaded) {
        generator->generate(chunk->voxels, x, z);
        chunkFlags.unsaved = true;
    }
    chunk->updateHeights();

    if (!chunkFlags.loadedLights) {
        Lighting::prebuildSkyLight(chunk.get(), level.content->getIndices());
    }
    chunkFlags.loaded = true;
    chunkFlags.ready = true;
    scripting::on_chunk_loaded(x, z);
}
