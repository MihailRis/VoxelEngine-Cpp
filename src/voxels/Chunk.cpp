#include "Chunk.hpp"

#include <utility>

#include "content/ContentReport.hpp"
#include "items/Inventory.hpp"
#include "lighting/Lightmap.hpp"
#include "voxel.hpp"

Chunk::Chunk(int xpos, int zpos) : x(xpos), z(zpos) {
    bottom = 0;
    top = CHUNK_H;
}

bool Chunk::isEmpty() {
    int id = -1;
    for (uint i = 0; i < CHUNK_VOL; i++) {
        if (voxels[i].id != id) {
            if (id != -1)
                return false;
            else
                id = voxels[i].id;
        }
    }
    return true;
}

void Chunk::updateHeights() {
    for (uint i = 0; i < CHUNK_VOL; i++) {
        if (voxels[i].id != 0) {
            bottom = i / (CHUNK_D * CHUNK_W);
            break;
        }
    }
    for (int i = CHUNK_VOL - 1; i >= 0; i--) {
        if (voxels[i].id != 0) {
            top = i / (CHUNK_D * CHUNK_W) + 1;
            break;
        }
    }
}

void Chunk::addBlockInventory(
    std::shared_ptr<Inventory> inventory, uint x, uint y, uint z
) {
    inventories[vox_index(x, y, z)] = std::move(inventory);
    flags.unsaved = true;
}

void Chunk::removeBlockInventory(uint x, uint y, uint z) {
    if (inventories.erase(vox_index(x, y, z))) {
        flags.unsaved = true;
    }
}

void Chunk::setBlockInventories(chunk_inventories_map map) {
    inventories = std::move(map);
}

std::shared_ptr<Inventory> Chunk::getBlockInventory(uint x, uint y, uint z)
    const {
    if (x >= CHUNK_W || y >= CHUNK_H || z >= CHUNK_D) return nullptr;
    const auto& found = inventories.find(vox_index(x, y, z));
    if (found == inventories.end()) {
        return nullptr;
    }
    return found->second;
}

std::unique_ptr<Chunk> Chunk::clone() const {
    auto other = std::make_unique<Chunk>(x, z);
    for (uint i = 0; i < CHUNK_VOL; i++) {
        other->voxels[i] = voxels[i];
    }
    other->lightmap.set(&lightmap);
    return other;
}

/**
  Current chunk format:
    - byte-order: big-endian
    - [don't panic!] first and second bytes are separated for RLE efficiency

    ```cpp
    uint8_t voxel_id_first_byte[CHUNK_VOL];
    uint8_t voxel_id_second_byte[CHUNK_VOL];
    uint8_t voxel_states_first_byte[CHUNK_VOL];
    uint8_t voxel_states_second_byte[CHUNK_VOL];
    ```

    Total size: (CHUNK_VOL * 4) bytes
*/
std::unique_ptr<ubyte[]> Chunk::encode() const {
    auto buffer = std::make_unique<ubyte[]>(CHUNK_DATA_LEN);
    for (uint i = 0; i < CHUNK_VOL; i++) {
        buffer[i] = voxels[i].id >> 8;
        buffer[CHUNK_VOL + i] = voxels[i].id & 0xFF;

        blockstate_t state = blockstate2int(voxels[i].state);
        buffer[CHUNK_VOL * 2 + i] = state >> 8;
        buffer[CHUNK_VOL * 3 + i] = state & 0xFF;
    }
    return buffer;
}

bool Chunk::decode(const ubyte* data) {
    for (uint i = 0; i < CHUNK_VOL; i++) {
        voxel& vox = voxels[i];

        ubyte bid1 = data[i];
        ubyte bid2 = data[CHUNK_VOL + i];

        ubyte bst1 = data[CHUNK_VOL * 2 + i];
        ubyte bst2 = data[CHUNK_VOL * 3 + i];

        vox.id =
            (static_cast<blockid_t>(bid1) << 8) | static_cast<blockid_t>(bid2);
        vox.state = int2blockstate(
            (static_cast<blockstate_t>(bst1) << 8) |
            static_cast<blockstate_t>(bst2)
        );
    }
    return true;
}

void Chunk::convert(ubyte* data, const ContentReport* report) {
    for (uint i = 0; i < CHUNK_VOL; i++) {
        // see encode method to understand what the hell is going on here
        blockid_t id =
            ((static_cast<blockid_t>(data[i]) << 8) |
             static_cast<blockid_t>(data[CHUNK_VOL + i]));
        blockid_t replacement = report->blocks.getId(id);
        data[i] = replacement >> 8;
        data[CHUNK_VOL + i] = replacement & 0xFF;
    }
}
