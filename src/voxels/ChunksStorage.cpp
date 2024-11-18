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
#include "VoxelsVolume.hpp"

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

static void check_voxels(const ContentIndices& indices, Chunk* chunk) {
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        blockid_t id = chunk->voxels[i].id;
        if (indices.blocks.get(id) == nullptr) {
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
        const auto& indices = *level->content->getIndices();

        chunk->decode(data.get());
        check_voxels(indices, chunk.get());

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

// reduce nesting on next modification
// 25.06.2024: not now
// TODO: move to Chunks for performance improvement
void ChunksStorage::getVoxels(VoxelsVolume* volume, bool backlight) const {
    const Content* content = level->content;
    auto indices = content->getIndices();
    voxel* voxels = volume->getVoxels();
    light_t* lights = volume->getLights();
    int x = volume->getX();
    int y = volume->getY();
    int z = volume->getZ();

    int w = volume->getW();
    int h = volume->getH();
    int d = volume->getD();

    int scx = floordiv(x, CHUNK_W);
    int scz = floordiv(z, CHUNK_D);

    int ecx = floordiv(x + w, CHUNK_W);
    int ecz = floordiv(z + d, CHUNK_D);

    int cw = ecx - scx + 1;
    int cd = ecz - scz + 1;

    // cw*cd chunks will be scanned
    for (int cz = scz; cz < scz + cd; cz++) {
        for (int cx = scx; cx < scx + cw; cx++) {
            const auto& found = chunksMap.find(glm::ivec2(cx, cz));
            if (found == chunksMap.end()) {
                // no chunk loaded -> filling with BLOCK_VOID
                for (int ly = y; ly < y + h; ly++) {
                    for (int lz = std::max(z, cz * CHUNK_D);
                             lz < std::min(z + d, (cz + 1) * CHUNK_D);
                             lz++) {
                        for (int lx = std::max(x, cx * CHUNK_W);
                                 lx < std::min(x + w, (cx + 1) * CHUNK_W);
                                 lx++) {
                            uint idx = vox_index(lx - x, ly - y, lz - z, w, d);
                            voxels[idx].id = BLOCK_VOID;
                            lights[idx] = 0;
                        }
                    }
                }
            } else {
                auto& chunk = found->second;
                const voxel* cvoxels = chunk->voxels;
                const light_t* clights = chunk->lightmap.getLights();
                for (int ly = y; ly < y + h; ly++) {
                    for (int lz = std::max(z, cz * CHUNK_D);
                             lz < std::min(z + d, (cz + 1) * CHUNK_D);
                             lz++) {
                        for (int lx = std::max(x, cx * CHUNK_W);
                                 lx < std::min(x + w, (cx + 1) * CHUNK_W);
                                 lx++) {
                            uint vidx = vox_index(lx - x, ly - y, lz - z, w, d);
                            uint cidx = vox_index(
                                lx - cx * CHUNK_W,
                                ly,
                                lz - cz * CHUNK_D,
                                CHUNK_W,
                                CHUNK_D
                            );
                            voxels[vidx] = cvoxels[cidx];
                            light_t light = clights[cidx];
                            if (backlight) {
                                const auto block =
                                    indices->blocks.get(voxels[vidx].id);
                                if (block && block->lightPassing) {
                                    light = Lightmap::combine(
                                        std::min(15,
                                            Lightmap::extract(light, 0) + 1),
                                        std::min(15,
                                            Lightmap::extract(light, 1) + 1),
                                        std::min(15,
                                            Lightmap::extract(light, 2) + 1),
                                        std::min(15, 
                                            static_cast<int>(Lightmap::extract(light, 3)))
                                    );
                                }
                            }
                            lights[vidx] = light;
                        }
                    }
                }
            }
        }
    }
}
