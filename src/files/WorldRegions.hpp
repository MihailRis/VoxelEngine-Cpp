#ifndef FILES_WORLD_REGIONS_HPP_
#define FILES_WORLD_REGIONS_HPP_

#include <condition_variable>
#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

#include <data/dynamic_fwd.hpp>
#include <typedefs.hpp>
#include <util/BufferPool.hpp>
#include <voxels/Chunk.hpp>
#include "files.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

namespace fs = std::filesystem;

inline constexpr uint REGION_HEADER_SIZE = 10;

inline constexpr uint REGION_LAYER_VOXELS = 0;
inline constexpr uint REGION_LAYER_LIGHTS = 1;
inline constexpr uint REGION_LAYER_INVENTORIES = 2;
inline constexpr uint REGION_LAYER_ENTITIES = 3;

inline constexpr uint REGION_SIZE_BIT = 5;
inline constexpr uint REGION_SIZE = (1 << (REGION_SIZE_BIT));
inline constexpr uint REGION_CHUNKS_COUNT = ((REGION_SIZE) * (REGION_SIZE));
inline constexpr uint REGION_FORMAT_VERSION = 2;
inline constexpr uint MAX_OPEN_REGION_FILES = 16;

class illegal_region_format : public std::runtime_error {
public:
    illegal_region_format(const std::string& message)
        : std::runtime_error(message) {
    }
};

class WorldRegion {
    std::unique_ptr<std::unique_ptr<ubyte[]>[]> chunksData;
    std::unique_ptr<uint32_t[]> sizes;
    bool unsaved = false;
public:
    WorldRegion();
    ~WorldRegion();

    void put(uint x, uint z, ubyte* data, uint32_t size);
    ubyte* getChunkData(uint x, uint z);
    uint getChunkDataSize(uint x, uint z);

    void setUnsaved(bool unsaved);
    bool isUnsaved() const;

    std::unique_ptr<ubyte[]>* getChunks() const;
    uint32_t* getSizes() const;
};

struct regfile {
    files::rafile file;
    int version;
    bool inUse = false;

    regfile(fs::path filename);
    regfile(const regfile&) = delete;

    std::unique_ptr<ubyte[]> read(int index, uint32_t& length);
};

using regionsmap = std::unordered_map<glm::ivec2, std::unique_ptr<WorldRegion>>;
using regionproc = std::function<bool(ubyte*)>;

struct RegionsLayer {
    int layer;
    fs::path folder;
    regionsmap regions;
    std::mutex mutex;
};

class regfile_ptr {
    regfile* file;
    std::condition_variable* cv;
public:
    regfile_ptr(regfile* file, std::condition_variable* cv)
        : file(file), cv(cv) {
    }

    regfile_ptr(const regfile_ptr&) = delete;

    regfile_ptr(std::nullptr_t) : file(nullptr), cv(nullptr) {
    }

    bool operator==(std::nullptr_t) const {
        return file == nullptr;
    }
    bool operator!=(std::nullptr_t) const {
        return file != nullptr;
    }
    operator bool() const {
        return file != nullptr;
    }
    ~regfile_ptr() {
        reset();
    }
    regfile* get() {
        return file;
    }
    void reset() {
        if (file) {
            file->inUse = false;
            cv->notify_one();
            file = nullptr;
        }
    }
};

class WorldRegions {
    fs::path directory;
    std::unordered_map<glm::ivec3, std::unique_ptr<regfile>> openRegFiles;
    std::mutex regFilesMutex;
    std::condition_variable regFilesCv;
    RegionsLayer layers[4] {};
    util::BufferPool<ubyte> bufferPool {
        std::max(CHUNK_DATA_LEN, LIGHTMAP_DATA_LEN) * 2};

    WorldRegion* getRegion(int x, int z, int layer);
    WorldRegion* getOrCreateRegion(int x, int z, int layer);

    /// @brief Compress buffer with extrle
    /// @param src source buffer
    /// @param srclen length of the source buffer
    /// @param len (out argument) length of result buffer
    /// @return compressed bytes array
    std::unique_ptr<ubyte[]> compress(
        const ubyte* src, size_t srclen, size_t& len
    );

    /// @brief Decompress buffer with extrle
    /// @param src compressed buffer
    /// @param srclen length of compressed buffer
    /// @param dstlen max expected length of source buffer
    /// @return decompressed bytes array
    std::unique_ptr<ubyte[]> decompress(
        const ubyte* src, size_t srclen, size_t dstlen
    );

    std::unique_ptr<ubyte[]> readChunkData(
        int x, int y, uint32_t& length, regfile* file
    );

    void fetchChunks(WorldRegion* region, int x, int y, regfile* file);

    ubyte* getData(int x, int z, int layer, uint32_t& size);

    regfile_ptr getRegFile(glm::ivec3 coord, bool create = true);
    void closeRegFile(glm::ivec3 coord);
    regfile_ptr useRegFile(glm::ivec3 coord);
    regfile_ptr createRegFile(glm::ivec3 coord);

    fs::path getRegionFilename(int x, int y) const;

    void writeRegions(int layer);

    /// @brief Write or rewrite region file
    /// @param x region X
    /// @param z region Z
    /// @param layer regions layer
    void writeRegion(int x, int y, int layer, WorldRegion* entry);
public:
    bool generatorTestMode = false;
    bool doWriteLights = true;

    WorldRegions(const fs::path& directory);
    WorldRegions(const WorldRegions&) = delete;
    ~WorldRegions();

    /// @brief Put all chunk data to regions
    void put(Chunk* chunk, std::vector<ubyte> entitiesData);

    /// @brief Store data in specified region
    /// @param x chunk.x
    /// @param z chunk.z
    /// @param layer regions layer
    /// @param data target data
    /// @param size data size
    /// @param rle compress with ext-RLE
    void put(
        int x,
        int z,
        int layer,
        std::unique_ptr<ubyte[]> data,
        size_t size,
        bool rle
    );

    std::unique_ptr<ubyte[]> getChunk(int x, int z);
    std::unique_ptr<light_t[]> getLights(int x, int z);
    chunk_inventories_map fetchInventories(int x, int z);
    dynamic::Map_sptr fetchEntities(int x, int z);

    void processRegionVoxels(int x, int z, const regionproc& func);

    fs::path getRegionsFolder(int layer) const;

    void write();

    /// @brief Extract X and Z from 'X_Z.bin' region file name.
    /// @param name source region file name
    /// @param x parsed X destination
    /// @param z parsed Z destination
    /// @return false if std::invalid_argument or std::out_of_range occurred
    static bool parseRegionFilename(const std::string& name, int& x, int& y);
};

#endif  // FILES_WORLD_REGIONS_HPP_
