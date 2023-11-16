#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <string>
#include <unordered_map>
#include <string>
#include <filesystem>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "../typedefs.h"

#define REGION_SIZE_BIT 5
#define REGION_SIZE (1 << (REGION_SIZE_BIT))
#define REGION_VOL ((REGION_SIZE) * (REGION_SIZE))
#define REGION_FORMAT_VERSION 1
#define REGION_FORMAT_MAGIC ".VOXREG"
#define WORLD_FORMAT_MAGIC ".VOXWLD"
#define WORLD_FORMAT_VERSION 1

class Player;
class Chunk;

struct WorldRegion {
	ubyte** chunksData;
	uint32_t* compressedSizes;
	bool unsaved;
};

struct WorldInfo {
	std::string name;
	std::filesystem::path directory;
	uint64_t seed;
};

class WorldFiles {
	void writeWorldInfo(const WorldInfo& info);
	std::filesystem::path getRegionFile(int x, int y) const;
	std::filesystem::path getPlayerFile() const;
	std::filesystem::path getWorldFile() const;
public:
	std::unordered_map<glm::ivec2, WorldRegion> regions;
	std::filesystem::path directory;
	ubyte* compressionBuffer;
	bool generatorTestMode;

	WorldFiles(std::filesystem::path directory, bool generatorTestMode);
	~WorldFiles();

	void put(Chunk* chunk);

	bool readWorldInfo(WorldInfo& info);
	bool readPlayer(Player* player);
	ubyte* readChunkData(int x, int y, uint32_t& length);
	ubyte* getChunk(int x, int y);
	void writeRegion(int x, int y, WorldRegion& entry);
	void writePlayer(Player* player);
	void write(const WorldInfo info);
};

#endif /* FILES_WORLDFILES_H_ */