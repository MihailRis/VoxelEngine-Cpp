#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <unordered_map>
#include <string>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "../typedefs.h"

class Player;

#define REGION_SIZE_BIT 5
#define REGION_SIZE (1 << (REGION_SIZE_BIT))
#define REGION_VOL ((REGION_SIZE) * (REGION_SIZE))

struct WorldRegion {
	ubyte** chunksData;
	bool unsaved;
};

class WorldFiles {
public:
	static int64_t totalCompressed;
	std::unordered_map<glm::ivec2, WorldRegion> regions;
	std::string directory;
	ubyte* mainBufferIn;
	ubyte* mainBufferOut;

	WorldFiles(std::string directory, size_t mainBufferCapacity);
	~WorldFiles();

	void put(const ubyte* chunkData, int x, int y);

	bool readPlayer(Player* player);
	bool readChunk(int x, int y, ubyte* out);
	bool getChunk(int x, int y, ubyte* out);
	uint writeRegion(ubyte* out, int x, int y, ubyte** region);
	void writePlayer(Player* player);
	void write();

	std::string getRegionFile(int x, int y);
	std::string getPlayerFile();
};

#endif /* FILES_WORLDFILES_H_ */
