#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>
#include <memory>
#include <glm/glm.hpp>
#include "../typedefs.h"

class VoxelRenderer;

class Chunk;
class voxel;
class WorldFiles;

class Chunks {
public:
	std::shared_ptr<Chunk>* chunks;
	std::shared_ptr<Chunk>* chunksSecond;
	size_t volume;
	size_t chunksCount;
	int w,d;
	int ox,oz;

	Chunks(int w, int d, int ox, int oz);
	~Chunks();

	bool putChunk(std::shared_ptr<Chunk> chunk);

	Chunk* getChunk(int x, int z);
	Chunk* getChunkByVoxel(int x, int y, int z);
	voxel* get(int x, int y, int z);
	light_t getLight(int x, int y, int z);
	ubyte getLight(int x, int y, int z, int channel);
	void set(int x, int y, int z, int id, uint8_t states);
	voxel* rayCast(glm::vec3 start, glm::vec3 dir, float maxLength, glm::vec3& end, glm::vec3& norm, glm::vec3& iend);

	bool isObstacle(int x, int y, int z);

	// does not move chunks inside
	void _setOffset(int x, int z);

	void setCenter(WorldFiles* worldFiles, int x, int z);
	void translate(WorldFiles* worldFiles, int x, int z);

	void clear();
};

#endif /* VOXELS_CHUNKS_H_ */
