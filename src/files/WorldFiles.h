#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <string>
#include <memory>
#include <unordered_map>
#include <filesystem>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#include "files.h"
#include "../typedefs.h"
#include "../settings.h"

const uint REGION_HEADER_SIZE = 10;
const uint REGION_LAYER_VOXELS = 0;
const uint REGION_LAYER_LIGHTS = 1;
const uint REGION_SIZE_BIT = 5;
const uint REGION_SIZE = (1 << (REGION_SIZE_BIT));
const uint REGION_CHUNKS_COUNT = ((REGION_SIZE) * (REGION_SIZE));
const uint REGION_FORMAT_VERSION = 2;
const uint WORLD_FORMAT_VERSION = 1;
const uint MAX_OPEN_REGION_FILES = 16;

#define REGION_FORMAT_MAGIC ".VOXREG"
#define WORLD_FORMAT_MAGIC ".VOXWLD"

class Player;
class Chunk;
class Content;
class ContentIndices;
class World;

namespace fs = std::filesystem;

class illegal_region_format : public std::runtime_error {
public:
    illegal_region_format(const std::string& message) 
    : std::runtime_error(message) {}
};

class WorldRegion {
	ubyte** chunksData;
	uint32_t* sizes;
	bool unsaved = false;
public:
	WorldRegion();
	~WorldRegion();

	void put(uint x, uint z, ubyte* data, uint32_t size);
	ubyte* getChunkData(uint x, uint z);
	uint getChunkDataSize(uint x, uint z);

	void setUnsaved(bool unsaved);
	bool isUnsaved() const;

	ubyte** getChunks() const;
	uint32_t* getSizes() const;
};

struct regfile {
    files::rafile file;
    int version;

    regfile(fs::path filename);
};

typedef std::unordered_map<glm::ivec2, std::unique_ptr<WorldRegion>> regionsmap;

class WorldFiles {
    std::unordered_map<glm::ivec3, std::unique_ptr<regfile>> openRegFiles;

	void writeWorldInfo(const World* world);
	fs::path getLightsFolder() const;
	fs::path getRegionFilename(int x, int y) const;
	fs::path getWorldFile() const;
	fs::path getIndicesFile() const;
	fs::path getPacksFile() const;
	
	WorldRegion* getRegion(regionsmap& regions, int x, int z);
	WorldRegion* getOrCreateRegion(regionsmap& regions, int x, int z);

	/* Compress buffer with extrle
	   @param src source buffer
	   @param srclen length of source buffer
	   @param len (out argument) length of result buffer */
	ubyte* compress(const ubyte* src, size_t srclen, size_t& len);

	/* Decompress buffer with extrle
	   @param src compressed buffer
	   @param srclen length of compressed buffer
	   @param dstlen max expected length of source buffer */
	ubyte* decompress(const ubyte* src, size_t srclen, size_t dstlen);

	ubyte* readChunkData(int x, int y, 
						 uint32_t& length, 
						 fs::path folder,
                         int layer);
    void fetchChunks(WorldRegion* region, int x, int y, 
                     fs::path folder, int layer);

	void writeRegions(regionsmap& regions,
					  const fs::path& folder, int layer);

	ubyte* getData(regionsmap& regions,
				   const fs::path& folder,
				   int x, int z, int layer);
    
    regfile* getRegFile(glm::ivec3 coord, const fs::path& folder);
public:
    static bool parseRegionFilename(const std::string& name, int& x, int& y);
    fs::path getRegionsFolder() const;
    fs::path getPlayerFile() const;

	regionsmap regions;
	regionsmap lights;
	fs::path directory;
	std::unique_ptr<ubyte[]> compressionBuffer;
	bool generatorTestMode;
	bool doWriteLights;

	WorldFiles(fs::path directory, const DebugSettings& settings);
	~WorldFiles();

	void put(Chunk* chunk);
    void put(int x, int z, const ubyte* voxelData);

    int getVoxelRegionVersion(int x, int z);
    int getVoxelRegionsVersion();

	ubyte* getChunk(int x, int z);
	light_t* getLights(int x, int z);

	bool readWorldInfo(World* world);
	bool readPlayer(Player* player);

	void writeRegion(int x, int y, 
					 WorldRegion* entry, 
					 fs::path file,
                     int layer);
	void writePlayer(Player* player);
    /* @param world world info to save (nullable) */
	void write(const World* world, const Content* content);
	void writePacks(const World* world);
	void writeIndices(const ContentIndices* indices);
    /* Append pack to packs.list without duplicate check */
    void addPack(const std::string& id);

    static const char* WORLD_FILE;
};

#endif /* FILES_WORLDFILES_H_ */