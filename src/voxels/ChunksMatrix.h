#ifndef VOXELS_CHUNKS_H_
#define VOXELS_CHUNKS_H_

#include <stdlib.h>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "../settings.h"
#include "../typedefs.h"

class VoxelRenderer;

class Chunk;
class Level;
class WorldFiles;
class LevelEvents;
class ChunksController;

/// @brief Chunk matrix loads chunks around it's center in certain radius
class ChunksMatrix {
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
