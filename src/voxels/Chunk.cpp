#include "Chunk.h"

#include <memory>

#include "voxel.h"
#include "../content/ContentLUT.h"
#include "../lighting/Lightmap.h"

Chunk::Chunk(int xpos, int zpos) : x(xpos), z(zpos){
	bottom = 0;
	top = CHUNK_H;
	voxels = new voxel[CHUNK_VOL];
	for (size_t i = 0; i < CHUNK_VOL; i++) {
		voxels[i].id = 2;
		voxels[i].states = 0;
	}
	lightmap = new Lightmap();
	renderData.vertices = nullptr;
}

Chunk::~Chunk(){
	delete lightmap;
	delete[] voxels;
}

bool Chunk::isEmpty(){
	int id = -1;
	for (size_t i = 0; i < CHUNK_VOL; i++){
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
	for (size_t i = 0; i < CHUNK_VOL; i++) {
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

Chunk* Chunk::clone() const {
	Chunk* other = new Chunk(x,z);
	for (size_t i = 0; i < CHUNK_VOL; i++)
		other->voxels[i] = voxels[i];
	other->lightmap->set(lightmap);
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
u_char8* Chunk::encode() const {
	u_char8* buffer = new u_char8[CHUNK_DATA_LEN];
	for (size_t i = 0; i < CHUNK_VOL; i++) {
		buffer[i] = voxels[i].id >> 8;
        buffer[CHUNK_VOL+i] = voxels[i].id & 0xFF;
		buffer[CHUNK_VOL*2 + i] = voxels[i].states >> 8;
        buffer[CHUNK_VOL*3 + i] = voxels[i].states & 0xFF;
	}
	return buffer;
}

/**
/**
 * @return true if all is fine
 **/
bool Chunk::decode(u_char8* data) {
	for (size_t i = 0; i < CHUNK_VOL; i++) {
		voxel& vox = voxels[i];

        u_char8 bid1 = data[i];
        u_char8 bid2 = data[CHUNK_VOL + i];
        
        u_char8 bst1 = data[CHUNK_VOL*2 + i];
        u_char8 bst2 = data[CHUNK_VOL*3 + i];

		vox.id = (blockid_t(bid1) << 8) | (blockid_t(bid2));
        vox.states = (blockstate_t(bst1) << 8) | (blockstate_t(bst2));
	}
	return true;
}

/*
 * Convert chunk voxels data from 16 bit to 32 bit
 */
void Chunk::fromOld(u_char8* data) {
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        data[i + CHUNK_VOL*3] = data[i + CHUNK_VOL];
        data[i + CHUNK_VOL] = data[i];
        data[i + CHUNK_VOL*2] = 0;
        data[i] = 0;
    }
}

void Chunk::convert(u_char8* data, const ContentLUT* lut) {
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        // see encode method to understand what the hell is going on here
        blockid_t id = ((blockid_t(data[i]) << 8) | 
                         blockid_t(data[CHUNK_VOL+i]));
        blockid_t replacement = lut->getBlockId(id);
        data[i] = replacement >> 8;
        data[CHUNK_VOL+i] = replacement & 0xFF;
    }
}
