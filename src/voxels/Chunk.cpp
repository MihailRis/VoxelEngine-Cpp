#include "Chunk.h"
#include "voxel.h"
#include "../lighting/Lightmap.h"

Chunk::Chunk(int xpos, int zpos) : x(xpos), z(zpos),
	top(CHUNK_H),
	bottom(0)
{
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
	for (int i = 0; i < CHUNK_VOL; i++){
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
	for (int i = 0; i < CHUNK_VOL; i++) {
		if (voxels[i].id != 0) {
			bottom = i / (CHUNK_D * CHUNK_W);
			break;
		}
	}

	for (int i = CHUNK_VOL - 1; i > -1; i--) {
		if (voxels[i].id != 0) {
			top = i / (CHUNK_D * CHUNK_W) + 1;
			break;
		}
	}
}

Chunk* Chunk::clone() const {
	Chunk* other = new Chunk(x,z);
	for (int i = 0; i < CHUNK_VOL; i++)
		other->voxels[i] = voxels[i];
	other->lightmap->set(lightmap);
	return other;
}

/** 
  Current chunk format:
	[voxel_ids...][voxel_states...];
*/
ubyte* Chunk::encode() const {
	ubyte* buffer = new ubyte[CHUNK_DATA_LEN];
	for (size_t i = 0; i < CHUNK_VOL; i++) {
		buffer[i] = voxels[i].id;
		buffer[CHUNK_VOL + i] = voxels[i].states;
	}
	return buffer;
}

/**
  @return true if all is fine
*/
bool Chunk::decode(ubyte* data) {
	for (size_t i = 0; i < CHUNK_VOL; i++) {
		voxel& vox = voxels[i];
		vox.id = data[i];
		vox.states = data[CHUNK_VOL + i];
	}
	return true;
}
