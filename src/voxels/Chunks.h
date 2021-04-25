#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>
#include <glm/glm.hpp>

using namespace glm;

class Mesh;
class VoxelRenderer;

class Chunk;
class voxel;
class WorldFiles;

class Chunks {
public:
	Chunk** chunks;
	Chunk** chunksSecond;
	Mesh** meshes;
	Mesh** meshesSecond;
	size_t volume;
	unsigned int w,h,d;
	int ox,oy,oz;

	Chunks(int w, int h, int d, int ox, int oy, int oz);
	~Chunks();

	Chunk* getChunk(int x, int y, int z);
	Chunk* getChunkByVoxel(int x, int y, int z);
	voxel* get(int x, int y, int z);
	unsigned char getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id);
	voxel* rayCast(vec3 start, vec3 dir, float maxLength, vec3& end, vec3& norm, vec3& iend);

	void setCenter(int x, int y, int z);
	void translate(int x, int y, int z);

	bool loadVisible(WorldFiles* worldFiles);
	bool _buildMeshes(VoxelRenderer* renderer);
};

#endif /* VOXELS_CHUNKS_H_ */
