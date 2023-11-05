#ifndef VOXELS_CHUNKSSTORAGE_H_
#define VOXELS_CHUNKSSTORAGE_H_

#include <memory>
#include <unordered_map>
#include "voxel.h"
#include "../typedefs.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

class Chunk;
class VoxelsVolume;

class ChunksStorage {
	std::unordered_map<glm::ivec2, std::shared_ptr<Chunk>> chunksMap;
public:
	ChunksStorage();
	virtual ~ChunksStorage();

	std::shared_ptr<Chunk> get(int x, int y) const;
	void store(std::shared_ptr<Chunk> chunk);
	void remove(int x, int y);
	void getVoxels(VoxelsVolume* volume) const;

	light_t getLight(int x, int y, int z, ubyte channel) const;
};


#endif // VOXELS_CHUNKSSTORAGE_H_