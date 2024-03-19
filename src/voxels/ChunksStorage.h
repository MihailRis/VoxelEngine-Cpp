#ifndef VOXELS_CHUNKSSTORAGE_H_
#define VOXELS_CHUNKSSTORAGE_H_

#include <memory>
#include <unordered_map>
#include "voxel.h"
#include "../typedefs.h"
#include "../maths/aabb.h"
#include "../content/Content.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

class Chunk;
class Level;
class VoxelsVolume;
class LevelEvents;

using ChunkMap = std::unordered_map<glm::ivec2, std::shared_ptr<Chunk>>;

class ChunksStorage {
	Level* level;
	const ContentIndices* const contentIds;
	ChunkMap chunksMap;
public:
	LevelEvents* events;

	ChunksStorage(Level* level);
	~ChunksStorage() = default;

	void store(std::shared_ptr<Chunk> chunk);
	void remove(int32_t x, int32_t y);
	std::shared_ptr<Chunk> create(int32_t x, int32_t z);
	void getVoxels(VoxelsVolume* volume, bool backlight=false) const;
	void save();
    void unloadUnused();

    inline ChunkMap::iterator begin() { return chunksMap.begin(); }
    inline ChunkMap::iterator end() { return chunksMap.end(); }

	Chunk* getChunk(int32_t x, int32_t z) const;
    Chunk* getChunkByVoxel(int32_t x, int32_t y, int32_t z) const;
    voxel* getVoxel(int32_t x, int32_t y, int32_t z) const;
	void setVoxel(int32_t x, int32_t y, int32_t z, blockid_t id, uint8_t states);
	light_t getLight(int32_t x, int32_t y, int32_t z);
	ubyte getLight(int32_t x, int32_t y, int32_t z, int channel);

	const AABB* isObstacleAt(float x, float y, float z);
    bool isSolidBlock(int32_t x, int32_t y, int32_t z);
    bool isReplaceableBlock(int32_t x, int32_t y, int32_t z);
	bool isObstacleBlock(int32_t x, int32_t y, int32_t z);

	voxel* rayCast(glm::vec3 start, 
				   glm::vec3 dir, 
				   float maxLength, 
				   glm::vec3& end, 
				   glm::ivec3& norm, 
				   glm::ivec3& iend);

	glm::vec3 rayCastToObstacle(glm::vec3 start, glm::vec3 dir, float maxDist);
};


#endif // VOXELS_CHUNKSSTORAGE_H_
