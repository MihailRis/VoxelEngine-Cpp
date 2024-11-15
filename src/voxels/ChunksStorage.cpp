#include "ChunksStorage.hpp"

#include <algorithm>

#include "content/Content.hpp"
#include "debug/Logger.hpp"
#include "files/WorldFiles.hpp"
#include "items/Inventories.hpp"
#include "lighting/Lightmap.hpp"
#include "maths/voxmaths.hpp"
#include "objects/Entities.hpp"
#include "typedefs.hpp"
#include "world/Level.hpp"
#include "world/World.hpp"
#include "Block.hpp"
#include "Chunk.hpp"

static debug::Logger logger("chunks-storage");

ChunksStorage::ChunksStorage(Level* level) : level(level) {
}

void ChunksStorage::store(const std::shared_ptr<Chunk>& chunk) {
    chunksMap[glm::ivec2(chunk->x, chunk->z)] = chunk;
}

std::shared_ptr<Chunk> ChunksStorage::get(int x, int z) const {
    auto found = chunksMap.find(glm::ivec2(x, z));
    if (found == chunksMap.end()) {
        return nullptr;
    }
    return found->second;
}

void ChunksStorage::remove(int x, int z) {
    auto found = chunksMap.find(glm::ivec2(x, z));
    if (found != chunksMap.end()) {
        chunksMap.erase(found->first);
    }
}

static void verifyLoadedChunk(ContentIndices* indices, Chunk* chunk) {
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        blockid_t id = chunk->voxels[i].id;
        if (indices->blocks.get(id) == nullptr) {
            auto logline = logger.error();
            logline << "corruped block detected at " << i << " of chunk ";
            logline << chunk->x << "x" << chunk->z;
            logline << " -> " << id;
            chunk->voxels[i].id = BLOCK_AIR;
        }
    }
}

std::shared_ptr<Chunk> ChunksStorage::create(int x, int z) {
    World* world = level->getWorld();
    auto& regions = world->wfile.get()->getRegions();

    auto chunk = std::make_shared<Chunk>(x, z);
    store(chunk);
    if (auto data = regions.getVoxels(chunk->x, chunk->z)) {
        chunk->decode(data.get());

        auto invs = regions.fetchInventories(chunk->x, chunk->z);
        chunk->setBlockInventories(std::move(invs));

        auto entitiesData = regions.fetchEntities(chunk->x, chunk->z);
        if (entitiesData.getType() == dv::value_type::object) {
            level->entities->loadEntities(std::move(entitiesData));
            chunk->flags.entities = true;
        }

        chunk->flags.loaded = true;
        for (auto& entry : chunk->inventories) {
            level->inventories->store(entry.second);
        }
        verifyLoadedChunk(level->content->getIndices(), chunk.get());
    }
    if (auto lights = regions.getLights(chunk->x, chunk->z)) {
        chunk->lightmap.set(lights.get());
        chunk->flags.loadedLights = true;
    }
    chunk->blocksMetadata = regions.getBlocksData(chunk->x, chunk->z);
    return chunk;
}
