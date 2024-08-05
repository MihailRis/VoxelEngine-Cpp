#include "WorldRegions.hpp"

#include <cstring>
#include <utility>
#include <vector>

#include <coders/byte_utils.hpp>
#include <coders/rle.hpp>
#include <data/dynamic.hpp>
#include <items/Inventory.hpp>
#include "../maths/voxmaths.hpp"
#include <util/data_io.hpp>

#define REGION_FORMAT_MAGIC ".VOXREG"

regfile::regfile(fs::path filename) : file(std::move(filename)) {
    if (file.length() < REGION_HEADER_SIZE)
        throw std::runtime_error("incomplete region file header");
    char header[REGION_HEADER_SIZE];
    file.read(header, REGION_HEADER_SIZE);

    // avoid of use strcmp_s
    if (std::string(header, strlen(REGION_FORMAT_MAGIC)) !=
        REGION_FORMAT_MAGIC) {
        throw std::runtime_error("invalid region file magic number");
    }
    version = header[8];
    if (uint(version) > REGION_FORMAT_VERSION) {
        throw illegal_region_format(
            "region format " + std::to_string(version) + " is not supported"
        );
    }
}

std::unique_ptr<ubyte[]> regfile::read(int index, uint32_t& length) {
    size_t file_size = file.length();
    size_t table_offset = file_size - REGION_CHUNKS_COUNT * 4;

    uint32_t offset;
    file.seekg(table_offset + index * 4);
    file.read(reinterpret_cast<char*>(&offset), 4);
    offset = dataio::read_int32_big(reinterpret_cast<const ubyte*>(&offset), 0);
    if (offset == 0) {
        return nullptr;
    }

    file.seekg(offset);
    file.read(reinterpret_cast<char*>(&offset), 4);
    length = dataio::read_int32_big(reinterpret_cast<const ubyte*>(&offset), 0);
    auto data = std::make_unique<ubyte[]>(length);
    file.read(reinterpret_cast<char*>(data.get()), length);
    return data;
}

WorldRegion::WorldRegion()
    : chunksData(
          std::make_unique<std::unique_ptr<ubyte[]>[]>(REGION_CHUNKS_COUNT)
      ),
      sizes(std::make_unique<uint32_t[]>(REGION_CHUNKS_COUNT)) {
}

WorldRegion::~WorldRegion() = default;

void WorldRegion::setUnsaved(bool unsaved) {
    this->unsaved = unsaved;
}
bool WorldRegion::isUnsaved() const {
    return unsaved;
}

std::unique_ptr<ubyte[]>* WorldRegion::getChunks() const {
    return chunksData.get();
}

uint32_t* WorldRegion::getSizes() const {
    return sizes.get();
}

void WorldRegion::put(uint x, uint z, ubyte* data, uint32_t size) {
    size_t chunk_index = z * REGION_SIZE + x;
    chunksData[chunk_index].reset(data);
    sizes[chunk_index] = size;
}

ubyte* WorldRegion::getChunkData(uint x, uint z) {
    return chunksData[z * REGION_SIZE + x].get();
}

uint WorldRegion::getChunkDataSize(uint x, uint z) {
    return sizes[z * REGION_SIZE + x];
}

WorldRegions::WorldRegions(const fs::path& directory) : directory(directory) {
    for (size_t i = 0; i < sizeof(layers) / sizeof(RegionsLayer); i++) {
        layers[i].layer = i;
    }
    layers[REGION_LAYER_VOXELS].folder = directory / fs::path("regions");
    layers[REGION_LAYER_LIGHTS].folder = directory / fs::path("lights");
    layers[REGION_LAYER_INVENTORIES].folder =
        directory / fs::path("inventories");
    layers[REGION_LAYER_ENTITIES].folder = directory / fs::path("entities");
}

WorldRegions::~WorldRegions() = default;

WorldRegion* WorldRegions::getRegion(int x, int z, int layer) {
    RegionsLayer& regions = layers[layer];
    std::lock_guard lock(regions.mutex);
    auto found = regions.regions.find(glm::ivec2(x, z));
    if (found == regions.regions.end()) {
        return nullptr;
    }
    return found->second.get();
}

WorldRegion* WorldRegions::getOrCreateRegion(int x, int z, int layer) {
    if (auto region = getRegion(x, z, layer)) {
        return region;
    }
    RegionsLayer& regions = layers[layer];
    std::lock_guard lock(regions.mutex);
    auto region_ptr = std::make_unique<WorldRegion>();
    auto region = region_ptr.get();
    regions.regions[{x, z}] = std::move(region_ptr);
    return region;
}

std::unique_ptr<ubyte[]> WorldRegions::compress(
    const ubyte* src, size_t srclen, size_t& len
) {
    auto buffer = bufferPool.get();
    auto bytes = buffer.get();

    len = extrle::encode(src, srclen, bytes);
    auto data = std::make_unique<ubyte[]>(len);
    for (size_t i = 0; i < len; i++) {
        data[i] = bytes[i];
    }
    return data;
}

std::unique_ptr<ubyte[]> WorldRegions::decompress(
    const ubyte* src, size_t srclen, size_t dstlen
) {
    auto decompressed = std::make_unique<ubyte[]>(dstlen);
    extrle::decode(src, srclen, decompressed.get());
    return decompressed;
}

inline void calc_reg_coords(
    int x, int z, int& regionX, int& regionZ, int& localX, int& localZ
) {
    regionX = floordiv(x, REGION_SIZE);
    regionZ = floordiv(z, REGION_SIZE);
    localX = x - (regionX * REGION_SIZE);
    localZ = z - (regionZ * REGION_SIZE);
}

std::unique_ptr<ubyte[]> WorldRegions::readChunkData(
    int x, int z, uint32_t& length, regfile* rfile
) {
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);
    int chunkIndex = localZ * REGION_SIZE + localX;
    return rfile->read(chunkIndex, length);
}

/// @brief Read missing chunks data (null pointers) from region file
void WorldRegions::fetchChunks(
    WorldRegion* region, int x, int z, regfile* file
) {
    auto* chunks = region->getChunks();
    uint32_t* sizes = region->getSizes();

    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        int chunk_x = (i % REGION_SIZE) + x * REGION_SIZE;
        int chunk_z = (i / REGION_SIZE) + z * REGION_SIZE;
        if (chunks[i] == nullptr) {
            chunks[i] = readChunkData(chunk_x, chunk_z, sizes[i], file);
        }
    }
}

ubyte* WorldRegions::getData(int x, int z, int layer, uint32_t& size) {
    if (generatorTestMode) {
        return nullptr;
    }
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

    WorldRegion* region = getOrCreateRegion(regionX, regionZ, layer);
    ubyte* data = region->getChunkData(localX, localZ);
    if (data == nullptr) {
        auto regfile = getRegFile(glm::ivec3(regionX, regionZ, layer));
        if (regfile != nullptr) {
            data = readChunkData(x, z, size, regfile.get()).release();
        }
        if (data != nullptr) {
            region->put(localX, localZ, data, size);
        }
    }
    if (data != nullptr) {
        size = region->getChunkDataSize(localX, localZ);
        return data;
    }
    return nullptr;
}

regfile_ptr WorldRegions::useRegFile(glm::ivec3 coord) {
    auto* file = openRegFiles[coord].get();
    file->inUse = true;
    return regfile_ptr(file, &regFilesCv);
}

void WorldRegions::closeRegFile(glm::ivec3 coord) {
    openRegFiles.erase(coord);
    regFilesCv.notify_one();
}

// Marks regfile as used and unmarks when shared_ptr dies
regfile_ptr WorldRegions::getRegFile(glm::ivec3 coord, bool create) {
    {
        std::lock_guard lock(regFilesMutex);
        const auto found = openRegFiles.find(coord);
        if (found != openRegFiles.end()) {
            if (found->second->inUse) {
                throw std::runtime_error("regfile is currently in use");
            }
            return useRegFile(found->first);
        }
    }
    if (create) {
        return createRegFile(coord);
    }
    return nullptr;
}

regfile_ptr WorldRegions::createRegFile(glm::ivec3 coord) {
    fs::path file =
        layers[coord[2]].folder / getRegionFilename(coord[0], coord[1]);
    if (!fs::exists(file)) {
        return nullptr;
    }
    if (openRegFiles.size() == MAX_OPEN_REGION_FILES) {
        std::unique_lock lock(regFilesMutex);
        while (true) {
            bool closed = false;
            // FIXME: bad choosing algorithm
            for (auto& entry : openRegFiles) {
                if (!entry.second->inUse) {
                    closeRegFile(entry.first);
                    closed = true;
                    break;
                }
            }
            if (closed) {
                break;
            }
            // notified when any regfile gets out of use or closed
            regFilesCv.wait(lock);
        }
        openRegFiles[coord] = std::make_unique<regfile>(file);
        return useRegFile(coord);
    } else {
        std::lock_guard lock(regFilesMutex);
        openRegFiles[coord] = std::make_unique<regfile>(file);
        return useRegFile(coord);
    }
}

fs::path WorldRegions::getRegionFilename(int x, int z) const {
    return fs::path(std::to_string(x) + "_" + std::to_string(z) + ".bin");
}

void WorldRegions::writeRegion(int x, int z, int layer, WorldRegion* entry) {
    fs::path filename = layers[layer].folder / getRegionFilename(x, z);

    glm::ivec3 regcoord(x, z, layer);
    if (auto regfile = getRegFile(regcoord, false)) {
        fetchChunks(entry, x, z, regfile.get());

        std::lock_guard lock(regFilesMutex);
        regfile.reset();
        closeRegFile(regcoord);
    }

    char header[REGION_HEADER_SIZE] = REGION_FORMAT_MAGIC;
    header[8] = REGION_FORMAT_VERSION;
    header[9] = 0;  // flags
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.write(header, REGION_HEADER_SIZE);

    size_t offset = REGION_HEADER_SIZE;
    char intbuf[4] {};
    uint offsets[REGION_CHUNKS_COUNT] {};

    auto* region = entry->getChunks();
    uint32_t* sizes = entry->getSizes();

    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        ubyte* chunk = region[i].get();
        if (chunk == nullptr) {
            offsets[i] = 0;
        } else {
            offsets[i] = offset;

            size_t compressedSize = sizes[i];
            dataio::write_int32_big(
                compressedSize, reinterpret_cast<ubyte*>(intbuf), 0
            );
            offset += 4 + compressedSize;

            file.write(intbuf, 4);
            file.write(reinterpret_cast<const char*>(chunk), compressedSize);
        }
    }
    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        dataio::write_int32_big(
            offsets[i], reinterpret_cast<ubyte*>(intbuf), 0
        );
        file.write(intbuf, 4);
    }
}

void WorldRegions::writeRegions(int layer) {
    for (auto& it : layers[layer].regions) {
        WorldRegion* region = it.second.get();
        if (region->getChunks() == nullptr || !region->isUnsaved()) {
            continue;
        }
        glm::ivec2 key = it.first;
        writeRegion(key[0], key[1], layer, region);
    }
}

void WorldRegions::put(
    int x,
    int z,
    int layer,
    std::unique_ptr<ubyte[]> data,
    size_t size,
    bool rle
) {
    if (rle) {
        size_t compressedSize;
        auto compressed = compress(data.get(), size, compressedSize);
        put(x, z, layer, std::move(compressed), compressedSize, false);
        return;
    }
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);

    WorldRegion* region = getOrCreateRegion(regionX, regionZ, layer);
    region->setUnsaved(true);
    region->put(localX, localZ, data.release(), size);
}

static std::unique_ptr<ubyte[]> write_inventories(
    Chunk* chunk, uint& datasize
) {
    auto& inventories = chunk->inventories;
    ByteBuilder builder;
    builder.putInt32(inventories.size());
    for (auto& entry : inventories) {
        builder.putInt32(entry.first);
        auto map = entry.second->serialize();
        auto bytes = json::to_binary(map.get(), true);
        builder.putInt32(bytes.size());
        builder.put(bytes.data(), bytes.size());
    }
    auto datavec = builder.data();
    datasize = builder.size();
    auto data = std::make_unique<ubyte[]>(datasize);
    std::memcpy(data.get(), datavec, datasize);
    return data;
}

/// @brief Store chunk data (voxels and lights) in region (existing or new)
void WorldRegions::put(Chunk* chunk, std::vector<ubyte> entitiesData) {
    assert(chunk != nullptr);
    if (!chunk->flags.lighted) {
        return;
    }
    bool lightsUnsaved = !chunk->flags.loadedLights && doWriteLights;
    if (!chunk->flags.unsaved && !lightsUnsaved && !chunk->flags.entities) {
        return;
    }

    int regionX, regionZ, localX, localZ;
    calc_reg_coords(chunk->x, chunk->z, regionX, regionZ, localX, localZ);

    put(chunk->x,
        chunk->z,
        REGION_LAYER_VOXELS,
        chunk->encode(),
        CHUNK_DATA_LEN,
        true);

    // Writing lights cache
    if (doWriteLights && chunk->flags.lighted) {
        put(chunk->x,
            chunk->z,
            REGION_LAYER_LIGHTS,
            chunk->lightmap.encode(),
            LIGHTMAP_DATA_LEN,
            true);
    }
    // Writing block inventories
    if (!chunk->inventories.empty()) {
        uint datasize;
        auto data = write_inventories(chunk, datasize);
        put(chunk->x,
            chunk->z,
            REGION_LAYER_INVENTORIES,
            std::move(data),
            datasize,
            false);
    }
    // Writing entities
    if (!entitiesData.empty()) {
        auto data = std::make_unique<ubyte[]>(entitiesData.size());
        for (size_t i = 0; i < entitiesData.size(); i++) {
            data[i] = entitiesData[i];
        }
        put(chunk->x,
            chunk->z,
            REGION_LAYER_ENTITIES,
            std::move(data),
            entitiesData.size(),
            false);
    }
}

std::unique_ptr<ubyte[]> WorldRegions::getChunk(int x, int z) {
    uint32_t size;
    auto* data = getData(x, z, REGION_LAYER_VOXELS, size);
    if (data == nullptr) {
        return nullptr;
    }
    return decompress(data, size, CHUNK_DATA_LEN);
}

/// @brief Get cached lights for chunk at x,z
/// @return lights data or nullptr
std::unique_ptr<light_t[]> WorldRegions::getLights(int x, int z) {
    uint32_t size;
    auto* bytes = getData(x, z, REGION_LAYER_LIGHTS, size);
    if (bytes == nullptr) {
        return nullptr;
    }
    auto data = decompress(bytes, size, LIGHTMAP_DATA_LEN);
    return Lightmap::decode(data.get());
}

chunk_inventories_map WorldRegions::fetchInventories(int x, int z) {
    chunk_inventories_map meta;
    uint32_t bytesSize;
    const ubyte* data = getData(x, z, REGION_LAYER_INVENTORIES, bytesSize);
    if (data == nullptr) {
        return meta;
    }
    ByteReader reader(data, bytesSize);
    auto count = reader.getInt32();
    for (int i = 0; i < count; i++) {
        uint index = reader.getInt32();
        uint size = reader.getInt32();
        auto map = json::from_binary(reader.pointer(), size);
        reader.skip(size);
        auto inv = std::make_shared<Inventory>(0, 0);
        inv->deserialize(map.get());
        meta[index] = inv;
    }
    return meta;
}

dynamic::Map_sptr WorldRegions::fetchEntities(int x, int z) {
    uint32_t bytesSize;
    const ubyte* data = getData(x, z, REGION_LAYER_ENTITIES, bytesSize);
    if (data == nullptr) {
        return nullptr;
    }
    auto map = json::from_binary(data, bytesSize);
    if (map->size() == 0) {
        return nullptr;
    }
    return map;
}

void WorldRegions::processRegionVoxels(int x, int z, const regionproc& func) {
    if (getRegion(x, z, REGION_LAYER_VOXELS)) {
        throw std::runtime_error("not implemented for in-memory regions");
    }
    auto regfile = getRegFile(glm::ivec3(x, z, REGION_LAYER_VOXELS));
    if (regfile == nullptr) {
        throw std::runtime_error("could not open region file");
    }
    for (uint cz = 0; cz < REGION_SIZE; cz++) {
        for (uint cx = 0; cx < REGION_SIZE; cx++) {
            int gx = cx + x * REGION_SIZE;
            int gz = cz + z * REGION_SIZE;
            uint32_t length;
            auto data = readChunkData(gx, gz, length, regfile.get());
            if (data == nullptr) {
                continue;
            }
            data = decompress(data.get(), length, CHUNK_DATA_LEN);
            if (func(data.get())) {
                put(gx,
                    gz,
                    REGION_LAYER_VOXELS,
                    std::move(data),
                    CHUNK_DATA_LEN,
                    true);
            }
        }
    }
}

fs::path WorldRegions::getRegionsFolder(int layer) const {
    return layers[layer].folder;
}

void WorldRegions::write() {
    for (auto& layer : layers) {
        fs::create_directories(layer.folder);
        writeRegions(layer.layer);
    }
}

bool WorldRegions::parseRegionFilename(
    const std::string& name, int& x, int& z
) {
    size_t sep = name.find('_');
    if (sep == std::string::npos || sep == 0 || sep == name.length() - 1) {
        return false;
    }
    try {
        x = std::stoi(name.substr(0, sep));
        z = std::stoi(name.substr(sep + 1));
    } catch (std::invalid_argument& err) {
        return false;
    } catch (std::out_of_range& err) {
        return false;
    }
    return true;
}
