#ifndef FILES_WORLDFILES_H_
#define FILES_WORLDFILES_H_

#include <map>
#include <array>
#include <vector>
#include <string>
#include <memory>
#include <optional>
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

class illegal_region_format : public std::runtime_error {
public:
    illegal_region_format(const std::string& message) 
    : std::runtime_error(message) {}
};

class WorldRegion {
    std::array<std::optional<std::vector<ubyte>>, REGION_CHUNKS_COUNT> chunks;
    bool unsaved = false;
public:
    WorldRegion() = default;
    ~WorldRegion() = default;

    void put(uint x, uint z, std::vector<ubyte>&& data);
    std::optional<std::vector<ubyte>>& getChunkData(uint x, uint z);

    void setUnsaved(bool unsaved);
    bool isUnsaved() const;

    std::array<std::optional<std::vector<ubyte>>, REGION_CHUNKS_COUNT>& getChunks() /*const*/;
};

struct regfile {
    files::rafile file;
    int version;

    regfile(std::filesystem::path filename);
};

using RegionsMap = std::unordered_map<glm::ivec2, std::unique_ptr<WorldRegion>>;
class WorldFiles {
    std::unordered_map<glm::ivec3, std::unique_ptr<regfile>> openRegFiles;

	void writeWorldInfo(const World* world);
	std::filesystem::path getLightsFolder() const;
	std::filesystem::path getRegionFilename(int x, int y) const;
	std::filesystem::path getPlayerFile() const;
	std::filesystem::path getWorldFile() const;
	std::filesystem::path getIndicesFile() const;
	std::filesystem::path getPacksFile() const;
	
	WorldRegion* getRegion(RegionsMap& regions,
						   int x, int z);

	WorldRegion* getOrCreateRegion(
						   RegionsMap& regions,
						   int x, int z);

    /* Compress buffer with extrle
       @param src source buffer
       @param srclen length of source buffer */
    std::vector<ubyte> compress(const ubyte* src, size_t srclen);

    /* Decompress buffer with extrle
       @param src compressed buffer
       @param dstlen max expected length of source buffer
    */
    std::vector<ubyte> decompress(const std::vector<ubyte>& src, size_t dstlen);

    std::optional<std::vector<ubyte>> readChunkData(int x, int y,
                                                    std::filesystem::path folder,
                                                    int layer);
    void fetchChunks(WorldRegion* region, int x, int y,
                     std::filesystem::path folder, int layer);

	void writeRegions(RegionsMap& regions,
					  const std::filesystem::path& folder, int layer);

    std::optional<std::vector<ubyte>> getData(RegionsMap& regions,
                                              const std::filesystem::path& folder,
                                              int x, int z, int layer);

    regfile* getRegFile(glm::ivec3 coord,
                        const std::filesystem::path& folder);
public:
    static bool parseRegionFilename(const std::string& name, int& x, int& y);
    std::filesystem::path getRegionsFolder() const;

	RegionsMap regions;
	RegionsMap lights;
	std::filesystem::path directory;
	bool generatorTestMode;
	bool doWriteLights;

	WorldFiles(std::filesystem::path directory, const DebugSettings& settings);
	~WorldFiles();

	void put(Chunk* chunk);
    void put(int x, int z, const ubyte* voxelData);

    int getVoxelRegionVersion(int x, int z);
    int getVoxelRegionsVersion();

    std::optional<std::vector<ubyte>> getChunk(int x, int z);
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