#include "Chunk.h"
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
u_char8* Chunk::encode() const {
	u_char8* buffer = new u_char8[CHUNK_DATA_LEN];
	for (size_t i = 0; i < CHUNK_VOL; i++) {
		((u_short16*)buffer)[i] = voxels[i].id;
		((u_short16*)buffer)[CHUNK_VOL + i] = voxels[i].states;
	}
	return buffer;
}

/**
  @return true if all is fine
*/
bool Chunk::decode(u_char8* data) {
	for (size_t i = 0; i < CHUNK_VOL; i++) {
		voxel& vox = voxels[i];
		vox.id = ((u_short16*)data)[i];
		vox.states = ((u_short16*)data)[CHUNK_VOL + i];
	}
	return true;
}

void Chunk::convert(u_char8* data, const ContentLUT* lut) {
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        blockid_t id = ((u_short16*)data)[i];
		((u_short16*)data)[i] = lut->getBlockId(id);
    }
}
