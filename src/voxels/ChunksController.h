#ifndef VOXELS_CHUNKSCONTROLLER_H_
#define VOXELS_CHUNKSCONTROLLER_H_

#include "../typedefs.h"

class Level;
class Chunks;
class Lighting;
class WorldFiles;
class VoxelRenderer;
class ChunksLoader;

class ChunksController {
private:
	Level* level;
	Chunks* chunks;
	Lighting* lighting;
	int64_t avgDurationMcs = 1000;
	uint padding;
public:
	ChunksController(Level* level, Chunks* chunks, Lighting* lighting, uint padding);
	~ChunksController();

    void update(int64_t maxDuration);
	bool loadVisible(WorldFiles* worldFiles);
};

#endif /* VOXELS_CHUNKSCONTROLLER_H_ */
