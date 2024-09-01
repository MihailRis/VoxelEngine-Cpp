#include "WorldRegions.hpp"

#include <cstring>
#include <utility>
#include <vector>

#include "coders/byte_utils.hpp"
#include "coders/rle.hpp"
#include "data/dynamic.hpp"
#include "items/Inventory.hpp"
#include "maths/voxmaths.hpp"
#include "util/data_io.hpp"

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

void WorldRegion::put(
    uint x, uint z, std::unique_ptr<ubyte[]> data, uint32_t size
) {
    size_t chunk_index = z * REGION_SIZE + x;
    chunksData[chunk_index] = std::move(data);
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

void RegionsLayer::writeAll() {
    for (auto& it : regions) {
        WorldRegion* region = it.second.get();
        if (region->getChunks() == nullptr || !region->isUnsaved()) {
            continue;
        }
        const auto& key = it.first;
        writeRegion(key[0], key[1], region);
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

    WorldRegion* region = layers[layer].getOrCreateRegion(regionX, regionZ);
    region->setUnsaved(true);
    region->put(localX, localZ, std::move(data), size);
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

std::unique_ptr<ubyte[]> WorldRegions::getVoxels(int x, int z) {
    uint32_t size;
    auto* data = layers[REGION_LAYER_VOXELS].getData(x, z, size);
    if (data == nullptr) {
        return nullptr;
    }
    return decompress(data, size, CHUNK_DATA_LEN);
}

/// @brief Get cached lights for chunk at x,z
/// @return lights data or nullptr
std::unique_ptr<light_t[]> WorldRegions::getLights(int x, int z) {
    uint32_t size;
    auto* bytes = layers[REGION_LAYER_LIGHTS].getData(x, z, size);
    if (bytes == nullptr) {
        return nullptr;
    }
    auto data = decompress(bytes, size, LIGHTMAP_DATA_LEN);
    return Lightmap::decode(data.get());
}

chunk_inventories_map WorldRegions::fetchInventories(int x, int z) {
    chunk_inventories_map meta;
    uint32_t bytesSize;
    auto bytes = layers[REGION_LAYER_INVENTORIES].getData(x, z, bytesSize);
    if (bytes == nullptr) {
        return meta;
    }
    ByteReader reader(bytes, bytesSize);
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
    if (generatorTestMode) {
        return nullptr;
    }
    uint32_t bytesSize;
    const ubyte* data = layers[REGION_LAYER_ENTITIES].getData(x, z, bytesSize);
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
    if (layers[REGION_LAYER_VOXELS].getRegion(x, z)) {
        throw std::runtime_error("not implemented for in-memory regions");
    }
    auto regfile = layers[REGION_LAYER_VOXELS].getRegFile({x, z});
    if (regfile == nullptr) {
        throw std::runtime_error("could not open region file");
    }
    for (uint cz = 0; cz < REGION_SIZE; cz++) {
        for (uint cx = 0; cx < REGION_SIZE; cx++) {
            int gx = cx + x * REGION_SIZE;
            int gz = cz + z * REGION_SIZE;
            uint32_t length;
            auto data =
                RegionsLayer::readChunkData(gx, gz, length, regfile.get());
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
        layer.writeAll();
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
