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

constexpr uint REGION_LAYER_VOXELS = 0;
constexpr uint REGION_LAYER_LIGHTS = 1;
constexpr uint REGION_SIZE_BIT = 5;
constexpr uint REGION_SIZE = (1 << (REGION_SIZE_BIT));
constexpr uint REGION_CHUNKS_COUNT = ((REGION_SIZE) * (REGION_SIZE));
constexpr uint REGION_FORMAT_VERSION = 1;
constexpr uint WORLD_FORMAT_VERSION = 1;
constexpr uint MAX_OPEN_REGION_FILES = 16;

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

typedef std::unordered_map<glm::ivec2, std::unique_ptr<WorldRegion>> regionsmap;
class WorldFiles {
    std::unordered_map<glm::ivec3, std::unique_ptr<files::rafile>> openRegFiles;

	void writeWorldInfo(const World* world);
	std::filesystem::path getLightsFolder() const;
	std::filesystem::path getRegionFilename(int x, int y) const;
	std::filesystem::path getPlayerFile() const;
	std::filesystem::path getWorldFile() const;
	std::filesystem::path getIndicesFile() const;
	std::filesystem::path getPacksFile() const;
	
	WorldRegion* getRegion(regionsmap& regions,
						   int x, int z);

	WorldRegion* getOrCreateRegion(
						   regionsmap& regions,
						   int x, int z);

	/* Compress buffer with extrle
	   @param src source buffer
	   @param srclen length of source buffer
	   @param len (out argument) length of result buffer */
	ubyte* compress(const ubyte* src, size_t srclen, size_t& len);

	/* Decompress buffer with extrle
	   @param src compressed buffer
	   @param srclen length of compressed buffer
	   @param dstlen max expected length of source buffer
	*/
	ubyte* decompress(const ubyte* src, size_t srclen, size_t dstlen);

	ubyte* readChunkData(int x, int y, 
						 uint32_t& length, 
						 std::filesystem::path folder,
                         int layer);
    void fetchChunks(WorldRegion* region, int x, int y, 
                     std::filesystem::path folder, int layer);

	void writeRegions(regionsmap& regions,
					  const std::filesystem::path& folder, int layer);

	ubyte* getData(regionsmap& regions,
				   const std::filesystem::path& folder,
				   int x, int z, int layer);
    
    files::rafile* getRegFile(glm::ivec3 coord,
                              const std::filesystem::path& folder);
public:
    static bool parseRegionFilename(const std::string& name, int& x, int& y);
    std::filesystem::path getRegionsFolder() const;

	regionsmap regions;
	regionsmap lights;
	std::filesystem::path directory;
	ubyte* compressionBuffer;
	bool generatorTestMode;
	bool doWriteLights;

	WorldFiles(std::filesystem::path directory, const DebugSettings& settings);
	~WorldFiles();

	void put(Chunk* chunk);
    void put(int x, int z, const ubyte* voxelData);

	ubyte* getChunk(int x, int z);
	light_t* getLights(int x, int z);

	bool readWorldInfo(World* world);
	bool readPlayer(Player* player);

	void writeRegion(int x, int y, 
					 WorldRegion* entry, 
					 std::filesystem::path file,
                     int layer);
	void writePlayer(Player* player);
    /* @param world world info to save (nullable) */
	void write(const World* world, const Content* content);
	void writePacks(const World* world);
	void writeIndices(const ContentIndices* indices);
};

#endif /* FILES_WORLDFILES_H_ */