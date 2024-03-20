#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "../typedefs.h"

class VoxelRenderer;

class Chunk;
class WorldFiles;
class LevelEvents;

/// @brief Chunk matrix, centered somewhere. Ensures, that chunks around it's center in certain radius are loaded
class ChunksMatrix {
public:
	std::vector<std::shared_ptr<Chunk>> chunks;
	std::vector<std::shared_ptr<Chunk>> chunksSecond;
	size_t volume;
	size_t chunksCount;
	size_t visible;
	uint32_t w, d;
	int32_t ox, oz;
    /// @brief Events here are local to loaded zone
    std::unique_ptr<LevelEvents> events;

	ChunksMatrix(uint32_t w, uint32_t d, int32_t ox, int32_t oz);
	~ChunksMatrix() = default;

	bool putChunk(std::shared_ptr<Chunk> chunk);

	// does not move chunks inside
	void _setOffset(int32_t x, int32_t z);

	void setCenter(int32_t x, int32_t z);
	void translate(int32_t x, int32_t z);
	void resize(uint32_t newW, uint32_t newD);

	void clear();
};

#endif /* VOXELS_CHUNKS_H_ */
