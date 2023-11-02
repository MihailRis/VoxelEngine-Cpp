#ifndef VOXELS_CHUNKSCONTROLLER_H_
#define VOXELS_CHUNKSCONTROLLER_H_

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
public:
	ChunksController(Level* level, Chunks* chunks, Lighting* lighting);
	~ChunksController();

	bool loadVisible(WorldFiles* worldFiles);
};

#endif /* VOXELS_CHUNKSCONTROLLER_H_ */
