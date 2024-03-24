#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "../settings.h"
#include "../typedefs.h"

class VoxelRenderer;

struct AABB;
struct voxel;
class Chunk;
class Level;
class WorldFiles;
class LevelEvents;
class ChunksController;
class ContentIndices;

/// @brief Chunk matrix loads chunks around it's center in certain radius
class ChunksMatrix {
    const ContentIndices* const contentIds;
public:
    std::unique_ptr<ChunksController> controller;
	std::vector<std::shared_ptr<Chunk>> chunks;
	std::vector<std::shared_ptr<Chunk>> chunksSecond;
	size_t volume;
	size_t chunksCount;
	size_t visible;
	uint32_t w, d;
	int32_t ox, oz;
    const EngineSettings& settings;
    /// @brief Events here are local to loaded zone
    std::unique_ptr<LevelEvents> events;

	ChunksMatrix(Level* level, uint32_t w, uint32_t d, int32_t ox, int32_t oz, const EngineSettings& settings);
	~ChunksMatrix() = default;

	bool putChunk(std::shared_ptr<Chunk> chunk);

	Chunk* getChunk(int32_t x, int32_t z);
	Chunk* getChunkByVoxel(int32_t x, int32_t y, int32_t z);
	voxel* getVoxel(int32_t x, int32_t y, int32_t z);
	light_t getLight(int32_t x, int32_t y, int32_t z);
	ubyte getLight(int32_t x, int32_t y, int32_t z, int channel);
	void setVoxel(int32_t x, int32_t y, int32_t z, uint32_t id, uint8_t states);

	voxel* rayCast(glm::vec3 start, 
				   glm::vec3 dir, 
				   float maxLength, 
				   glm::vec3& end, 
				   glm::ivec3& norm, 
				   glm::ivec3& iend);

	glm::vec3 rayCastToObstacle(glm::vec3 start, glm::vec3 dir, float maxDist);

	const AABB* isObstacleAt(float x, float y, float z);
    bool isSolidBlock(int32_t x, int32_t y, int32_t z);
    bool isReplaceableBlock(int32_t x, int32_t y, int32_t z);
	bool isObstacleBlock(int32_t x, int32_t y, int32_t z);
	// does not move chunks inside
	void _setOffset(int32_t x, int32_t z);

	void setCenter(int32_t x, int32_t z);
	void translate(int32_t x, int32_t z);
	void resize(uint32_t newW, uint32_t newD);
    void update();

	void clear();

    uint32_t getPadding() const;
};

#endif /* VOXELS_CHUNKS_H_ */
