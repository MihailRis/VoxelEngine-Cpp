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

ChunksStorage::ChunksStorage(Level* level)
    : level(level),
      chunksMap(std::make_shared<util::WeakPtrsMap<glm::ivec2, Chunk>>()) {
}

std::shared_ptr<Chunk> ChunksStorage::fetch(int x, int z) {
    std::lock_guard lock(*chunksMap);
    return chunksMap->fetch({x, z});
}

static void check_voxels(const ContentIndices& indices, Chunk& chunk) {
    bool corrupted = false;
    blockid_t defsCount = indices.blocks.count();
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        blockid_t id = chunk.voxels[i].id;
        if (id >= defsCount) {
            if (!corrupted) {
#ifdef NDEBUG
                // release
                auto logline = logger.error();
                logline << "corruped blocks detected at " << i << " of chunk ";
                logline << chunk.x << "x" << chunk.z;
                logline << " -> " << id;
                corrupted = true;
#else
                // debug
                abort();
#endif
            }
            chunk.voxels[i].id = BLOCK_AIR;
        }
    }
}

std::shared_ptr<Chunk> ChunksStorage::create(int x, int z) {
    if (auto ptr = chunksMap->fetch({x, z})) {
        return ptr;
    }

    World* world = level->getWorld();
    auto& regions = world->wfile.get()->getRegions();

    auto& localChunksMap = chunksMap;
    auto chunk = std::shared_ptr<Chunk>(
        new Chunk(x, z),
        [localChunksMap, x, z](Chunk* ptr) {
            std::lock_guard lock(*localChunksMap);
            localChunksMap->erase({x, z});
            delete ptr;
        }
    );
    (*chunksMap)[glm::ivec2(chunk->x, chunk->z)] = chunk;
    if (auto data = regions.getVoxels(chunk->x, chunk->z)) {
        const auto& indices = *level->content->getIndices();

        chunk->decode(data.get());
        check_voxels(indices, *chunk);
        auto invs = regions.fetchInventories(chunk->x, chunk->z);
        auto iterator = invs.begin();
        while (iterator != invs.end()) {
            uint index = iterator->first;
            const auto& def = indices.blocks.require(chunk->voxels[index].id);
            if (def.inventorySize == 0) {
                iterator = invs.erase(iterator);
                continue;
            }
            auto& inventory = iterator->second;
            if (def.inventorySize != inventory->size()) {
                inventory->resize(def.inventorySize);
            }
            ++iterator;
        }
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
    }
    if (auto lights = regions.getLights(chunk->x, chunk->z)) {
        chunk->lightmap.set(lights.get());
        chunk->flags.loadedLights = true;
    }
    chunk->blocksMetadata = regions.getBlocksData(chunk->x, chunk->z);
    return chunk;
}
