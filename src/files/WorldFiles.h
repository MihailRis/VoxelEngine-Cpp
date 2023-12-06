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

const uint REGION_SIZE_BIT = 5;
const uint REGION_SIZE = (1 << (REGION_SIZE_BIT));
const uint REGION_VOL = ((REGION_SIZE) * (REGION_SIZE));
const uint REGION_FORMAT_VERSION = 1;
const uint WORLD_FORMAT_VERSION = 1;
#define REGION_FORMAT_MAGIC ".VOXREG"
#define WORLD_FORMAT_MAGIC ".VOXWLD"

class Player;
class Chunk;
class Content;
class ContentIndices;
class World;

class WorldRegion {
	ubyte** chunksData;
	uint32_t* sizes;
	bool unsaved = true;
public:
	WorldRegion();
	~WorldRegion();

	void put(uint x, uint z, ubyte* data, uint32_t size);
	ubyte* get(uint x, uint z);
	uint getSize(uint x, uint z);

	void setUnsaved(bool unsaved);
	bool isUnsaved() const;

	ubyte** getChunks() const;
	uint32_t* getSizes() const;
};

class WorldFiles {
	void writeWorldInfo(const World* world);
	std::filesystem::path getRegionsFolder() const;
	std::filesystem::path getRegionFile(int x, int y) const;
	std::filesystem::path getPlayerFile() const;
	std::filesystem::path getWorldFile() const;
	std::filesystem::path getIndicesFile() const;

	// TODO: remove in 0.16
	std::filesystem::path getOldPlayerFile() const;
	std::filesystem::path getOldWorldFile() const;
	bool readOldWorldInfo(World* world);
	bool readOldPlayer(Player* player);
	// --------------------

	WorldRegion* getRegion(int x, int z);

	/* Compress buffer with extrle
	   @param src source buffer
	   @param srclen length of source buffer
	   @param len (out argument) length of result buffer */
	ubyte* compress(ubyte* src, size_t srclen, size_t& len);

	/* Decompress buffer with extrle
	   @param src compressed buffer
	   @param srclen length of compressed buffer
	   @param dstlen max expected length of source buffer
	*/
	ubyte* decompress(ubyte* src, size_t srclen, size_t dstlen);
public:
	std::unordered_map<glm::ivec2, WorldRegion*> regions;
	std::filesystem::path directory;
	ubyte* compressionBuffer;
	bool generatorTestMode;

	WorldFiles(std::filesystem::path directory, bool generatorTestMode);
	~WorldFiles();

	void put(Chunk* chunk);
	ubyte* getChunk(int x, int y);

	bool readWorldInfo(World* world);
	bool readPlayer(Player* player);
	ubyte* readChunkData(int x, int y, 
						 uint32_t& length, 
						 std::filesystem::path file);
	void writeRegion(int x, int y, 
					 WorldRegion* entry, 
					 std::filesystem::path file);
	void writePlayer(Player* player);
	void write(const World* world, const Content* content);
	void writeIndices(const ContentIndices* indices);
};

#endif /* FILES_WORLDFILES_H_ */