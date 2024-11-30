#include "BlocksController.hpp"

#include "content/Content.hpp"
#include "items/Inventories.hpp"
#include "items/Inventory.hpp"
#include "lighting/Lighting.hpp"
#include "maths/fastmaths.hpp"
#include "scripting/scripting.hpp"
#include "voxels/Block.hpp"
#include "voxels/Chunk.hpp"
#include "voxels/Chunks.hpp"
#include "voxels/voxel.hpp"
#include "world/Level.hpp"
#include "world/LevelEvents.hpp"

BlocksController::BlocksController(const Level& level, uint padding)
    : level(level),
      chunks(*level.chunks),
      lighting(*level.lighting),
      randTickClock(20, 3),
      blocksTickClock(20, 1),
      worldTickClock(20, 1),
      padding(padding) {
}

void BlocksController::updateSides(int x, int y, int z) {
    updateBlock(x - 1, y, z);
    updateBlock(x + 1, y, z);
    updateBlock(x, y - 1, z);
    updateBlock(x, y + 1, z);
    updateBlock(x, y, z - 1);
    updateBlock(x, y, z + 1);
}

void BlocksController::updateSides(int x, int y, int z, int w, int h, int d) {
    voxel* vox = chunks.get(x, y, z);
    const auto& def = level.content->getIndices()->blocks.require(vox->id);
    const auto& rot = def.rotations.variants[vox->state.rotation];
    const auto& xaxis = rot.axisX;
    const auto& yaxis = rot.axisY;
    const auto& zaxis = rot.axisZ;
    for (int ly = -1; ly <= h; ly++) {
        for (int lz = -1; lz <= d; lz++) {
            for (int lx = -1; lx <= w; lx++) {
                if (lx >= 0 && lx < w && ly >= 0 && ly < h && lz >= 0 && lz < d) {
                    continue;
                }
                updateBlock(
                    x + lx * xaxis.x + ly * yaxis.x + lz * zaxis.x,
                    y + lx * xaxis.y + ly * yaxis.y + lz * zaxis.y,
                    z + lx * xaxis.z + ly * yaxis.z + lz * zaxis.z
                );
            }
        }
    }
}

void BlocksController::breakBlock(
    Player* player, const Block& def, int x, int y, int z
) {
    glm::ivec3 pos(x, y, z);
    onBlockInteraction(
        player, pos, def, BlockInteraction::destruction
    );
    chunks.set(x, y, z, 0, {});
    lighting.onBlockSet(x, y, z, 0);
    level.events->trigger(EVT_BLOCK_CHANGED, &pos);
    scripting::on_block_broken(player, def, pos);
    if (def.rt.extended) {
        updateSides(x, y, z , def.size.x, def.size.y, def.size.z);
    } else {
        updateSides(x, y, z);
    }
}

void BlocksController::placeBlock(
    Player* player, const Block& def, blockstate state, int x, int y, int z
) {
    auto voxel = chunks.get(x, y, z);
    if (voxel == nullptr) {
        return;
    }
    glm::ivec3 pos(x, y, z);
    const auto& prevDef = level.content->getIndices()->blocks.require(voxel->id);
    level.events->trigger(EVT_BLOCK_CHANGED, &pos);
    scripting::on_block_replaced(player, prevDef, pos);

    onBlockInteraction(
        player, pos, def, BlockInteraction::placing
    );
    chunks.set(x, y, z, def.rt.id, state);
    lighting.onBlockSet(x, y, z, def.rt.id);
    scripting::on_block_placed(player, def, pos);
    if (def.rt.extended) {
        updateSides(x, y, z , def.size.x, def.size.y, def.size.z);
    } else {
        updateSides(x, y, z);
    }
}

void BlocksController::updateBlock(int x, int y, int z) {
    voxel* vox = chunks.get(x, y, z);
    if (vox == nullptr) return;
    const auto& def = level.content->getIndices()->blocks.require(vox->id);
    if (def.grounded) {
        const auto& vec = get_ground_direction(def, vox->state.rotation);
        if (!chunks.isSolidBlock(x + vec.x, y + vec.y, z + vec.z)) {
            breakBlock(nullptr, def, x, y, z);
            return;
        }
    }
    if (def.rt.funcsset.update) {
        scripting::update_block(def, glm::ivec3(x, y, z));
    }
}

void BlocksController::update(float delta) {
    if (randTickClock.update(delta)) {
        randomTick(randTickClock.getPart(), randTickClock.getParts());
    }
    if (blocksTickClock.update(delta)) {
        onBlocksTick(blocksTickClock.getPart(), blocksTickClock.getParts());
    }
    if (worldTickClock.update(delta)) {
        scripting::on_world_tick();
    }
}

void BlocksController::onBlocksTick(int tickid, int parts) {
    const auto& indices = level.content->getIndices()->blocks;
    int tickRate = blocksTickClock.getTickRate();
    for (size_t id = 0; id < indices.count(); id++) {
        if ((id + tickid) % parts != 0) continue;
        auto& def = indices.require(id);
        auto interval = def.tickInterval;
        if (def.rt.funcsset.onblockstick && tickid / parts % interval == 0) {
            scripting::on_blocks_tick(def, tickRate / interval);
        }
    }
}

void BlocksController::randomTick(
    const Chunk& chunk, int segments, const ContentIndices* indices
) {
    const int segheight = CHUNK_H / segments;

    for (int s = 0; s < segments; s++) {
        for (int i = 0; i < 4; i++) {
            int bx = random.rand() % CHUNK_W;
            int by = random.rand() % segheight + s * segheight;
            int bz = random.rand() % CHUNK_D;
            const voxel& vox = chunk.voxels[vox_index(bx, by, bz)];
            auto& block = indices->blocks.require(vox.id);
            if (block.rt.funcsset.randupdate) {
                scripting::random_update_block(
                    block,
                    glm::ivec3(
                        chunk.x * CHUNK_W + bx, by, chunk.z * CHUNK_D + bz
                    )
                );
            }
        }
    }
}

void BlocksController::randomTick(int tickid, int parts) {
    auto indices = level.content->getIndices();
    int width = chunks.getWidth();
    int height = chunks.getHeight();
    int segments = 4;

    for (uint z = padding; z < height - padding; z++) {
        for (uint x = padding; x < width - padding; x++) {
            int index = z * width + x;
            if ((index + tickid) % parts != 0) {
                continue;
            }
            auto& chunk = chunks.getChunks()[index];
            if (chunk == nullptr || !chunk->flags.lighted) {
                continue;
            }
            randomTick(*chunk, segments, indices);
        }
    }
}

int64_t BlocksController::createBlockInventory(int x, int y, int z) {
    auto chunk = chunks.getChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        return 0;
    }
    int lx = x - chunk->x * CHUNK_W;
    int lz = z - chunk->z * CHUNK_D;
    auto inv = chunk->getBlockInventory(lx, y, lz);
    if (inv == nullptr) {
        const auto& indices = level.content->getIndices()->blocks;
        auto& def = indices.require(chunk->voxels[vox_index(lx, y, lz)].id);
        int invsize = def.inventorySize;
        if (invsize == 0) {
            return 0;
        }
        inv = level.inventories->create(invsize);
        chunk->addBlockInventory(inv, lx, y, lz);
    }
    return inv->getId();
}

void BlocksController::bindInventory(int64_t invid, int x, int y, int z) {
    auto chunk = chunks.getChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        throw std::runtime_error("block does not exists");
    }
    if (invid <= 0) {
        throw std::runtime_error("unable to bind virtual inventory");
    }
    int lx = x - chunk->x * CHUNK_W;
    int lz = z - chunk->z * CHUNK_D;
    chunk->addBlockInventory(level.inventories->get(invid), lx, y, lz);
}

void BlocksController::unbindInventory(int x, int y, int z) {
    auto chunk = chunks.getChunkByVoxel(x, y, z);
    if (chunk == nullptr) {
        throw std::runtime_error("block does not exists");
    }
    int lx = x - chunk->x * CHUNK_W;
    int lz = z - chunk->z * CHUNK_D;
    chunk->removeBlockInventory(lx, y, lz);
}

void BlocksController::onBlockInteraction(
    Player* player, glm::ivec3 pos, const Block& def, BlockInteraction type
) {
    for (const auto& callback : blockInteractionCallbacks) {
        callback(player, pos, def, type);
    }
}

void BlocksController::listenBlockInteraction(
    const on_block_interaction& callback
) {
    blockInteractionCallbacks.push_back(callback);
}
