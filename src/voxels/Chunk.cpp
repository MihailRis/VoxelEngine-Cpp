#include "Chunk.h"

#include "voxel.h"

#include "../items/Inventory.h"
#include "../content/ContentLUT.h"
#include "../lighting/Lightmap.h"

Chunk::Chunk(int xpos, int zpos) : x(xpos), z(zpos){
	bottom = 0;
	top = CHUNK_H;
	for (uint i = 0; i < CHUNK_VOL; i++) {
		voxels[i].id = 2;
		voxels[i].states = 0;
	}
}

bool Chunk::isEmpty(){
	int id = -1;
	for (uint i = 0; i < CHUNK_VOL; i++){
		if (voxels[i].id != id){
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

void Chunk::addBlockInventory(std::shared_ptr<Inventory> inventory, 
                              uint x, uint y, uint z) {
    inventories[vox_index(x, y, z)] = inventory;
    setUnsaved(true);
}

std::shared_ptr<Inventory> Chunk::getBlockInventory(uint x, uint y, uint z) const {
    if (x >= CHUNK_W || y >= CHUNK_H || z >= CHUNK_D)
        return nullptr;
    const auto& found = inventories.find(vox_index(x, y, z));
    if (found == inventories.end()) {
        return nullptr;
    }
    return found->second;
}

std::unique_ptr<Chunk> Chunk::clone() const {
	auto other = std::make_unique<Chunk>(x,z);
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
ubyte* Chunk::encode() const {
	ubyte* buffer = new ubyte[CHUNK_DATA_LEN];
	for (uint i = 0; i < CHUNK_VOL; i++) {
		buffer[i] = voxels[i].id >> 8;
        buffer[CHUNK_VOL+i] = voxels[i].id & 0xFF;
		buffer[CHUNK_VOL*2 + i] = voxels[i].states >> 8;
        buffer[CHUNK_VOL*3 + i] = voxels[i].states & 0xFF;
	}
	return buffer;
}

bool Chunk::decode(const ubyte* data) {
	for (uint i = 0; i < CHUNK_VOL; i++) {
		voxel& vox = voxels[i];

        ubyte bid1 = data[i];
        ubyte bid2 = data[CHUNK_VOL + i];
        
        ubyte bst1 = data[CHUNK_VOL*2 + i];
        ubyte bst2 = data[CHUNK_VOL*3 + i];

		vox.id = (blockid_t(bid1) << 8) | (blockid_t(bid2));
        vox.states = (blockstate_t(bst1) << 8) | (blockstate_t(bst2));
	}
	return true;
}

void Chunk::convert(ubyte* data, const ContentLUT* lut) {
    for (uint i = 0; i < CHUNK_VOL; i++) {
        // see encode method to understand what the hell is going on here
        blockid_t id = ((blockid_t(data[i]) << 8) | 
                         blockid_t(data[CHUNK_VOL+i]));
        blockid_t replacement = lut->getBlockId(id);
        data[i] = replacement >> 8;
        data[CHUNK_VOL+i] = replacement & 0xFF;
    }
}
