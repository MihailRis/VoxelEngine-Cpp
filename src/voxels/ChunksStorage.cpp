#include "ChunksStorage.hpp"

#include <algorithm>

#include "content/Content.hpp"
#include "coders/json.hpp"
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

inline long long keyfrom(int x, int z) {
    union {
        int pos[2];
        long long key;
    } ekey;
    ekey.pos[0] = x;
    ekey.pos[1] = z;
    return ekey.key;
}

static debug::Logger logger("chunks-storage");

ChunksStorage::ChunksStorage(Level* level) : level(level) {
    chunksMap.max_load_factor(0.2f);
}

std::shared_ptr<Chunk> ChunksStorage::fetch(int x, int z) {
    const auto& found = chunksMap.find(keyfrom(x, z));
    if (found == chunksMap.end()) {
        return nullptr;
    }
    return found->second;
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

void ChunksStorage::erase(int x, int z) {
    chunksMap.erase(keyfrom(x, z));
}

std::shared_ptr<Chunk> ChunksStorage::create(int x, int z) {
    const auto& found = chunksMap.find(keyfrom(x, z));
    if (found != chunksMap.end()) {
        return found->second;
    }

    auto chunk = std::make_shared<Chunk>(x, z);
    chunksMap[keyfrom(x, z)] = chunk;

    World* world = level->getWorld();
    auto& regions = world->wfile.get()->getRegions();

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

void ChunksStorage::pinChunk(std::shared_ptr<Chunk> chunk) {
    pinnedChunks[{chunk->x, chunk->z}] = std::move(chunk);
}

void ChunksStorage::unpinChunk(int x, int z) {
    pinnedChunks.erase({x, z});
}

size_t ChunksStorage::size() const {
    return chunksMap.size();
}

voxel* ChunksStorage::get(int x, int y, int z) const {
    if (y < 0 || y >= CHUNK_H) {
        return nullptr;
    }

    int cx = floordiv<CHUNK_W>(x);
    int cz = floordiv<CHUNK_D>(z);

    const auto& found = chunksMap.find(keyfrom(cx, cz));
    if (found == chunksMap.end()) {
        return nullptr;
    }
    const auto& chunk = found->second;
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;
    return &chunk->voxels[(y * CHUNK_D + lz) * CHUNK_W + lx];
}

void ChunksStorage::incref(Chunk* chunk) {
    auto key = reinterpret_cast<ptrdiff_t>(chunk);
    const auto& found = refCounters.find(key);
    if (found == refCounters.end()) {
        refCounters[key] = 1;
        return;
    }
    found->second++;
}

void ChunksStorage::decref(Chunk* chunk) {
    auto key = reinterpret_cast<ptrdiff_t>(chunk);
    const auto& found = refCounters.find(key);
    if (found == refCounters.end()) {
        abort();
    }
    if (--found->second == 0) {
        union {
            int pos[2];
            long long key;
        } ekey;
        ekey.pos[0] = chunk->x;
        ekey.pos[1] = chunk->z;

        save(chunk);
        chunksMap.erase(ekey.key);
        refCounters.erase(found);
    }
}

void ChunksStorage::save(Chunk* chunk) {
    if (chunk == nullptr) {
        return;
    }
    AABB aabb(
        glm::vec3(chunk->x * CHUNK_W, -INFINITY, chunk->z * CHUNK_D),
        glm::vec3(
            (chunk->x + 1) * CHUNK_W, INFINITY, (chunk->z + 1) * CHUNK_D
        )
    );
    auto entities = level->entities->getAllInside(aabb);
    auto root = dv::object();
    root["data"] = level->entities->serialize(entities);
    if (!entities.empty()) {
        level->entities->despawn(std::move(entities));
        chunk->flags.entities = true;
    }
    level->getWorld()->wfile->getRegions().put(
        chunk,
        chunk->flags.entities ? json::to_binary(root, true)
                                : std::vector<ubyte>()
    );
}

void ChunksStorage::saveAll() {
    for (const auto& [_, chunk] : chunksMap) {
        save(chunk.get());
    }
}
