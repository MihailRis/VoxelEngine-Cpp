#include "Chunks.h"
#include "Chunk.h"
#include "voxel.h"
#include "Block.h"
#include "WorldGenerator.h"
#include "../lighting/Lightmap.h"
#include "../files/WorldFiles.h"

#include "../graphics/Mesh.h"

#include <math.h>
#include <limits.h>

Chunks::Chunks(int w, int d, int ox, int oz) : w(w), d(d), ox(ox), oz(oz){
	volume = w*d;
	chunks = new Chunk*[volume];
	chunksSecond = new Chunk*[volume];

	meshes = new Mesh*[volume];
	meshesSecond = new Mesh*[volume];

	for (size_t i = 0; i < volume; i++){
		chunks[i] = nullptr;
		meshes[i] = nullptr;
	}
	chunksCount = 0;
}

Chunks::~Chunks(){
	for (size_t i = 0; i < volume; i++){
		if (chunks[i])
			chunks[i]->decref();
	}
	delete[] chunks;
}

voxel* Chunks::get(int x, int y, int z){
	x -= ox * CHUNK_W;
	z -= oz * CHUNK_D;
	int cx = x / CHUNK_W;
	int cy = y / CHUNK_H;
	int cz = z / CHUNK_D;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= w || cy >= 1 || cz >= d)
		return nullptr;
	Chunk* chunk = chunks[(cy * d + cz) * w + cx];
	if (chunk == nullptr)
		return nullptr;
	int lx = x - cx * CHUNK_W;
	int ly = y - cy * CHUNK_H;
	int lz = z - cz * CHUNK_D;
	return &chunk->voxels[(ly * CHUNK_D + lz) * CHUNK_W + lx];
}

bool Chunks::isObstacle(int x, int y, int z){
	voxel* v = get(x,y,z);
	if (v == nullptr)
		return true; // void - is obstacle
	return Block::blocks[v->id]->obstacle;
}

unsigned char Chunks::getLight(int x, int y, int z, int channel){
	x -= ox * CHUNK_W;
	z -= oz * CHUNK_D;
	int cx = x / CHUNK_W;
	int cy = y / CHUNK_H;
	int cz = z / CHUNK_D;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= w || cy >= 1 || cz >= d)
		return 0;
	Chunk* chunk = chunks[(cy * d + cz) * w + cx];
	if (chunk == nullptr)
		return 0;
	int lx = x - cx * CHUNK_W;
	int ly = y - cy * CHUNK_H;
	int lz = z - cz * CHUNK_D;
	return chunk->lightmap->get(lx,ly,lz, channel);
}

unsigned short Chunks::getLight(int x, int y, int z){
	x -= ox * CHUNK_W;
	z -= oz * CHUNK_D;
	int cx = x / CHUNK_W;
	int cy = y / CHUNK_H;
	int cz = z / CHUNK_D;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= w || cy >= 1 || cz >= d)
		return 0;
	Chunk* chunk = chunks[(cy * d + cz) * w + cx];
	if (chunk == nullptr)
		return 0;
	int lx = x - cx * CHUNK_W;
	int ly = y - cy * CHUNK_H;
	int lz = z - cz * CHUNK_D;
	return chunk->lightmap->get(lx,ly,lz);
}

Chunk* Chunks::getChunkByVoxel(int x, int y, int z){
	x -= ox * CHUNK_W;
	z -= oz * CHUNK_D;
	int cx = x / CHUNK_W;
	int cy = y / CHUNK_H;
	int cz = z / CHUNK_D;
	if (x < 0) cx--;
	if (y < 0) cy--;
	if (z < 0) cz--;
	if (cx < 0 || cy < 0 || cz < 0 || cx >= w || cy >= 1 || cz >= d)
		return nullptr;
	return chunks[(cy * d + cz) * w + cx];
}

Chunk* Chunks::getChunk(int x, int z){
	x -= ox;
	z -= oz;
	if (x < 0 || z < 0 || x >= w || z >= d)
		return nullptr;
	return chunks[z * w + x];
}

void Chunks::set(int x, int y, int z, int id){
	x -= ox * CHUNK_W;
	z -= oz * CHUNK_D;
	int cx = x / CHUNK_W;
	if (y < 0 || y >= CHUNK_H)
		return;
	int cz = z / CHUNK_D;
	if (x < 0) cx--;
	if (z < 0) cz--;
	if (cx < 0 || cz < 0 || cx >= w || cz >= d)
		return;
	Chunk* chunk = chunks[cz * w + cx];
	if (chunk == nullptr)
		return;
	int lx = x - cx * CHUNK_W;
	int lz = z - cz * CHUNK_D;
	chunk->voxels[(y * CHUNK_D + lz) * CHUNK_W + lx].id = id;
	chunk->setUnsaved(true);
	chunk->setModified(true);

	if (lx == 0 && (chunk = getChunk(cx+ox-1, cz+oz))) chunk->setModified(true);
	if (lz == 0 && (chunk = getChunk(cx+ox, cz+oz-1))) chunk->setModified(true);

	if (lx == CHUNK_W-1 && (chunk = getChunk(cx+ox+1, cz+oz))) chunk->setModified(true);
	if (lz == CHUNK_D-1 && (chunk = getChunk(cx+ox, cz+oz+1))) chunk->setModified(true);
}

voxel* Chunks::rayCast(vec3 a, vec3 dir, float maxDist, vec3& end, vec3& norm, vec3& iend) {
	float px = a.x;
	float py = a.y;
	float pz = a.z;

	float dx = dir.x;
	float dy = dir.y;
	float dz = dir.z;

	float t = 0.0f;
	int ix = floor(px);
	int iy = floor(py);
	int iz = floor(pz);

	float stepx = (dx > 0.0f) ? 1.0f : -1.0f;
	float stepy = (dy > 0.0f) ? 1.0f : -1.0f;
	float stepz = (dz > 0.0f) ? 1.0f : -1.0f;

	float infinity = std::numeric_limits<float>::infinity();

	float txDelta = (dx == 0.0f) ? infinity : abs(1.0f / dx);
	float tyDelta = (dy == 0.0f) ? infinity : abs(1.0f / dy);
	float tzDelta = (dz == 0.0f) ? infinity : abs(1.0f / dz);

	float xdist = (stepx > 0) ? (ix + 1 - px) : (px - ix);
	float ydist = (stepy > 0) ? (iy + 1 - py) : (py - iy);
	float zdist = (stepz > 0) ? (iz + 1 - pz) : (pz - iz);

	float txMax = (txDelta < infinity) ? txDelta * xdist : infinity;
	float tyMax = (tyDelta < infinity) ? tyDelta * ydist : infinity;
	float tzMax = (tzDelta < infinity) ? tzDelta * zdist : infinity;

	int steppedIndex = -1;

	while (t <= maxDist){
		voxel* voxel = get(ix, iy, iz);
		if (voxel == nullptr || Block::blocks[voxel->id]->selectable){
			end.x = px + t * dx;
			end.y = py + t * dy;
			end.z = pz + t * dz;

			iend.x = ix;
			iend.y = iy;
			iend.z = iz;

			norm.x = norm.y = norm.z = 0.0f;
			if (steppedIndex == 0) norm.x = -stepx;
			if (steppedIndex == 1) norm.y = -stepy;
			if (steppedIndex == 2) norm.z = -stepz;
			return voxel;
		}
		if (txMax < tyMax) {
			if (txMax < tzMax) {
				ix += stepx;
				t = txMax;
				txMax += txDelta;
				steppedIndex = 0;
			} else {
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
				steppedIndex = 2;
			}
		} else {
			if (tyMax < tzMax) {
				iy += stepy;
				t = tyMax;
				tyMax += tyDelta;
				steppedIndex = 1;
			} else {
				iz += stepz;
				t = tzMax;
				tzMax += tzDelta;
				steppedIndex = 2;
			}
		}
	}
	iend.x = ix;
	iend.y = iy;
	iend.z = iz;

	end.x = px + t * dx;
	end.y = py + t * dy;
	end.z = pz + t * dz;
	norm.x = norm.y = norm.z = 0.0f;
	return nullptr;
}

void Chunks::setCenter(WorldFiles* worldFiles, int x, int z) {
	int cx = x / CHUNK_W;
	int cz = z / CHUNK_D;
	cx -= ox;
	cz -= oz;
	if (x < 0) cx--;
	if (z < 0) cz--;
	cx -= w/2;
	cz -= d/2;
	if (cx | cz) {
		translate(worldFiles, cx,cz);
	}
}

void Chunks::translate(WorldFiles* worldFiles, int dx, int dz){
	for (unsigned int i = 0; i < volume; i++){
		chunksSecond[i] = nullptr;
		meshesSecond[i] = nullptr;
	}
	for (int z = 0; z < d; z++){
		for (int x = 0; x < w; x++){
			Chunk* chunk = chunks[z * w + x];
			int nx = x - dx;
			int nz = z - dz;
			if (chunk == nullptr)
				continue;
			Mesh* mesh = meshes[z * w + x];
			if (nx < 0 || nz < 0 || nx >= w || nz >= d){
				worldFiles->put((const char*)chunk->voxels, chunk->x, chunk->z);
				chunk->decref();
				delete mesh;
				chunksCount--;
				continue;
			}
			meshesSecond[nz * w + nx] = mesh;
			chunksSecond[nz * w + nx] = chunk;
		}
	}
	Chunk** ctemp = chunks;
	chunks = chunksSecond;
	chunksSecond = ctemp;

	Mesh** mtemp = meshes;
	meshes = meshesSecond;
	meshesSecond = mtemp;

	ox += dx;
	oz += dz;
}

void Chunks::_setOffset(int x, int z){
	ox = x;
	oz = z;
}

bool Chunks::putChunk(Chunk* chunk) {
	int x = chunk->x;
	int z = chunk->z;
	x -= ox;
	z -= oz;
	if (x < 0 || z < 0 || x >= w || z >= d)
		return false;
	chunks[z * w + x] = chunk;
	chunksCount++;
	return true;
}

void Chunks::clear(bool freeMemory){
	for (size_t i = 0; i < volume; i++){
		if (freeMemory){
			chunks[i]->decref();
			delete meshes[i];
		}
		chunks[i] = nullptr;
		meshes[i] = nullptr;
	}
	chunksCount = 0;
}
