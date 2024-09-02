#pragma once

#include <condition_variable>
#include <filesystem>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "constants.hpp"
#include "typedefs.hpp"
#include "data/dynamic_fwd.hpp"
#include "util/BufferPool.hpp"
#include "voxels/Chunk.hpp"
#include "maths/voxmaths.hpp"
#include "coders/compression.hpp"
#include "files.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace fs = std::filesystem;

inline constexpr uint REGION_HEADER_SIZE = 10;

enum RegionLayerIndex : uint {
    REGION_LAYER_VOXELS = 0,
    REGION_LAYER_LIGHTS,
    REGION_LAYER_INVENTORIES,
    REGION_LAYER_ENTITIES,
    
    REGION_LAYERS_COUNT
};

inline constexpr uint REGION_SIZE_BIT = 5;
inline constexpr uint REGION_SIZE = (1 << (REGION_SIZE_BIT));
inline constexpr uint REGION_CHUNKS_COUNT = ((REGION_SIZE) * (REGION_SIZE));

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

    void put(uint x, uint z, std::unique_ptr<ubyte[]> data, uint32_t size);
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

inline void calc_reg_coords(
    int x, int z, int& regionX, int& regionZ, int& localX, int& localZ
) {
    regionX = floordiv(x, REGION_SIZE);
    regionZ = floordiv(z, REGION_SIZE);
    localX = x - (regionX * REGION_SIZE);
    localZ = z - (regionZ * REGION_SIZE);
}

struct RegionsLayer {
    /// @brief Layer index
    RegionLayerIndex layer;
    
    /// @brief Regions layer folder
    fs::path folder;

    compression::Method compression = compression::Method::NONE;

    /// @brief In-memory regions data
    regionsmap regions;

    /// @brief In-memory regions map mutex
    std::mutex mapMutex;

    /// @brief Open region files map
    std::unordered_map<glm::ivec2, std::unique_ptr<regfile>> openRegFiles;

    /// @brief Open region files map mutex
    std::mutex regFilesMutex;
    std::condition_variable regFilesCv;

    [[nodiscard]] regfile_ptr getRegFile(glm::ivec2 coord, bool create = true);
    [[nodiscard]] regfile_ptr useRegFile(glm::ivec2 coord);
    regfile_ptr createRegFile(glm::ivec2 coord);
    void closeRegFile(glm::ivec2 coord);

    WorldRegion* getRegion(int x, int z);
    WorldRegion* getOrCreateRegion(int x, int z);

    /// @brief Get chunk data. Read from file if not loaded yet.
    /// @param x chunk x coord
    /// @param z chunk z coord
    /// @param size [out] chunk data length
    /// @return nullptr if no saved chunk data found
    [[nodiscard]] ubyte* getData(int x, int z, uint32_t& size);

    /// @brief Write or rewrite region file
    /// @param x region X
    /// @param z region Z
    void writeRegion(int x, int y, WorldRegion* entry);

    /// @brief Write all unsaved regions to files
    void writeAll();

    /// @brief Read chunk data from region file
    /// @param x chunk x coord
    /// @param z chunk z coord
    /// @param length [out] chunk data length
    /// @param rfile region file
    /// @return nullptr if chunk is not present in region file
    [[nodiscard]] static std::unique_ptr<ubyte[]> readChunkData(
        int x, int z, uint32_t& length, regfile* rfile
    );
};

class WorldRegions {
    /// @brief World directory
    fs::path directory;

    RegionsLayer layers[REGION_LAYERS_COUNT] {};
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
    void put(
        int x,
        int z,
        RegionLayerIndex layer,
        std::unique_ptr<ubyte[]> data,
        size_t size
    );

    std::unique_ptr<ubyte[]> getVoxels(int x, int z);

    /// @brief Get cached lights for chunk at x,z
    /// @return lights data or nullptr
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
