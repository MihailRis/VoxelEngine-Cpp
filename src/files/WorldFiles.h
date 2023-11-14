#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <string>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "../typedefs.h"

#define REGION_SIZE_BIT 5
#define REGION_SIZE (1 << (REGION_SIZE_BIT))
#define REGION_VOL ((REGION_SIZE) * (REGION_SIZE))
#define REGION_FORMAT_VERSION 1

class Player;
class Chunk;

struct WorldRegion {
	ubyte** chunksData;
	uint32_t* compressedSizes;
	bool unsaved;
};

class WorldFiles {
public:
	std::unordered_map<glm::ivec2, WorldRegion> regions;
	std::string directory;
	ubyte* compressionBuffer;
	bool generatorTestMode;

	WorldFiles(std::string directory, size_t mainBufferCapacity, bool generatorTestMode);
	~WorldFiles();

	void put(Chunk* chunk);

	bool readPlayer(Player* player);
	ubyte* readChunkData(int x, int y, uint32_t& length);
	ubyte* getChunk(int x, int y);
	void writeRegion(int x, int y, WorldRegion& entry);
	void writePlayer(Player* player);
	void write();

	std::string getRegionFile(int x, int y);
	std::string getPlayerFile();
};

#endif /* FILES_WORLDFILES_H_ */
