#include "WorldFiles.h"

#include "../coders/byte_utils.h"
#include "../coders/json.h"
#include "../constants.h"
#include "../content/Content.h"
#include "../core_defs.h"
#include "../data/dynamic.h"
#include "../items/Inventory.h"
#include "../items/ItemDef.h"
#include "../lighting/Lightmap.h"
#include "../maths/voxmaths.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../typedefs.h"
#include "../util/data_io.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../voxels/voxel.h"
#include "../window/Camera.h"
#include "../world/World.h"
#include "rle.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cstring>

#define REGION_FORMAT_MAGIC ".VOXREG"
#define WORLD_FORMAT_MAGIC ".VOXWLD"

const size_t BUFFER_SIZE_UNKNOWN = -1;

regfile::regfile(fs::path filename) : file(filename) {
    if (file.length() < REGION_HEADER_SIZE)
        throw std::runtime_error("incomplete region file header");
    char header[REGION_HEADER_SIZE];
    file.read(header, REGION_HEADER_SIZE);
    
    // avoid of use strcmp_s
    if (std::string(header, strlen(REGION_FORMAT_MAGIC)) != REGION_FORMAT_MAGIC) {
        throw std::runtime_error("invalid region file magic number");
    }
    version = header[8];
    if (uint(version) > REGION_FORMAT_VERSION) {
        throw illegal_region_format(
            "region format "+std::to_string(version)+" is not supported"
        );
    }
}

WorldRegion::WorldRegion() {
    chunksData = new ubyte*[REGION_CHUNKS_COUNT]{};
    sizes = new uint32_t[REGION_CHUNKS_COUNT]{};
}

WorldRegion::~WorldRegion() {
    for (uint i = 0; i < REGION_CHUNKS_COUNT; i++) {
        delete[] chunksData[i];
    }
    delete[] sizes;
    delete[] chunksData;
}

void WorldRegion::setUnsaved(bool unsaved) {
    this->unsaved = unsaved;
}
bool WorldRegion::isUnsaved() const {
    return unsaved;
}

ubyte** WorldRegion::getChunks() const {
    return chunksData;
}

uint32_t* WorldRegion::getSizes() const {
    return sizes;
}

void WorldRegion::put(uint x, uint z, ubyte* data, uint32_t size) {
    size_t chunk_index = z * REGION_SIZE + x;
    delete[] chunksData[chunk_index];
    chunksData[chunk_index] = data;
    sizes[chunk_index] = size;
}

ubyte* WorldRegion::getChunkData(uint x, uint z) {
    return chunksData[z * REGION_SIZE + x];
}

uint WorldRegion::getChunkDataSize(uint x, uint z) {
    return sizes[z * REGION_SIZE + x];
}

WorldFiles::WorldFiles(fs::path directory) : directory(directory) {
    for (uint i = 0; i < sizeof(layers)/sizeof(RegionsLayer); i++) {
        layers[i].layer = i;
    }
    layers[REGION_LAYER_VOXELS].folder = directory/fs::path("regions");
    layers[REGION_LAYER_LIGHTS].folder = directory/fs::path("lights");
    layers[REGION_LAYER_INVENTORIES].folder = directory/fs::path("inventories");
}

WorldFiles::WorldFiles(fs::path directory, const DebugSettings& settings) 
  : WorldFiles(directory) 
{
    generatorTestMode = settings.generatorTestMode;
    doWriteLights = settings.doWriteLights;
}

WorldFiles::~WorldFiles() {
}

void WorldFiles::createDirectories() {
    fs::create_directories(directory / fs::path("data"));
    fs::create_directories(directory / fs::path("content"));
}

WorldRegion* WorldFiles::getRegion(int x, int z, int layer) {
    RegionsLayer& regions = layers[layer];
    std::lock_guard lock(regions.mutex);
    auto found = regions.regions.find(glm::ivec2(x, z));
    if (found == regions.regions.end())
        return nullptr;
    return found->second.get();
}

WorldRegion* WorldFiles::getOrCreateRegion(int x, int z, int layer) {
    RegionsLayer& regions = layers[layer];
    WorldRegion* region = getRegion(x, z, layer);
    if (region == nullptr) {
        std::lock_guard lock(regions.mutex);
        region = new WorldRegion();
        regions.regions[glm::ivec2(x, z)].reset(region);
    }
    return region;
}

std::unique_ptr<ubyte[]> WorldFiles::compress(const ubyte* src, size_t srclen, size_t& len) {
    auto buffer = bufferPool.get();
    ubyte* bytes = buffer.get();
    
    len = extrle::encode(src, srclen, bytes);
    auto data = std::make_unique<ubyte[]>(len);
    for (size_t i = 0; i < len; i++) {
        data[i] = bytes[i];
    }
    return data;
}

std::unique_ptr<ubyte[]> WorldFiles::decompress(const ubyte* src, size_t srclen, size_t dstlen) {
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

void WorldFiles::put(int x, int z, int layer, std::unique_ptr<ubyte[]> data, size_t size, bool rle) {
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

std::unique_ptr<ubyte[]> write_inventories(Chunk* chunk, uint& datasize) {
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
    for (uint i = 0; i < datasize; i++) {
        data[i] = datavec[i];
    }
    return data;
}

/// @brief Store chunk (voxels and lights) in region (existing or new)
void WorldFiles::put(Chunk* chunk){
    assert(chunk != nullptr);

    int regionX, regionZ, localX, localZ;
    calc_reg_coords(chunk->x, chunk->z, regionX, regionZ, localX, localZ);

    put(chunk->x, chunk->z, REGION_LAYER_VOXELS, 
        chunk->encode(), CHUNK_DATA_LEN, true);

    // Writing lights cache
    if (doWriteLights && chunk->isLighted()) {
        put(chunk->x, chunk->z, REGION_LAYER_LIGHTS, 
            chunk->lightmap.encode(), LIGHTMAP_DATA_LEN, true);
    }
    // Writing block inventories
    if (!chunk->inventories.empty()){
        uint datasize;
        put(chunk->x, chunk->z, REGION_LAYER_INVENTORIES, 
            write_inventories(chunk, datasize), datasize, false);
    }
}

fs::path WorldFiles::getRegionFilename(int x, int z) const {
    return fs::path(std::to_string(x) + "_" + std::to_string(z) + ".bin");
}

/// @brief Extract X and Z from 'X_Z.bin' region file name.
/// @param name source region file name
/// @param x parsed X destination
/// @param z parsed Z destination
/// @return false if std::invalid_argument or std::out_of_range occurred
bool WorldFiles::parseRegionFilename(const std::string& name, int& x, int& z) {
    size_t sep = name.find('_');
    if (sep == std::string::npos || sep == 0 || sep == name.length()-1)
        return false;
    try {
        x = std::stoi(name.substr(0, sep));
        z = std::stoi(name.substr(sep+1));
    } catch (std::invalid_argument& err) {
        return false;
    } catch (std::out_of_range& err) {
        return false;
    }
    return true;
}

fs::path WorldFiles::getPlayerFile() const {
    return directory/fs::path("player.json");
}

fs::path WorldFiles::getWorldFile() const {
    return directory/fs::path(WORLD_FILE);
}

fs::path WorldFiles::getIndicesFile() const {
    return directory/fs::path("indices.json");
}

fs::path WorldFiles::getPacksFile() const {
    return directory/fs::path("packs.list");
}

std::unique_ptr<ubyte[]> WorldFiles::getChunk(int x, int z){
    uint32_t size;
    auto* data = getData(x, z, REGION_LAYER_VOXELS, size);
    if (data == nullptr) {
        return nullptr;
    }
    return decompress(data, size, CHUNK_DATA_LEN);
}

/// @brief Get cached lights for chunk at x,z 
/// @return lights data or nullptr
std::unique_ptr<light_t[]> WorldFiles::getLights(int x, int z) {
    uint32_t size;
    auto* bytes = getData(x, z, REGION_LAYER_LIGHTS, size);
    if (bytes == nullptr)
        return nullptr;
    auto data = decompress(bytes, size, LIGHTMAP_DATA_LEN);
    return Lightmap::decode(data.get());
}

chunk_inventories_map WorldFiles::fetchInventories(int x, int z) {
    chunk_inventories_map inventories;
    uint32_t bytesSize;
    const ubyte* data = getData(x, z, REGION_LAYER_INVENTORIES, bytesSize);
    if (data == nullptr)
        return inventories;
    ByteReader reader(data, bytesSize);
    int count = reader.getInt32();
    for (int i = 0; i < count; i++) {
        uint index = reader.getInt32();
        uint size = reader.getInt32();
        auto map = json::from_binary(reader.pointer(), size);
        reader.skip(size);
        auto inv = std::make_shared<Inventory>(0, 0);
        inv->deserialize(map.get());
        inventories[index] = inv;
    }
    return inventories;
}

ubyte* WorldFiles::getData(
    int x, int z, int layer, 
    uint32_t& size
) {
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

std::shared_ptr<regfile> WorldFiles::useRegFile(glm::ivec3 coord) {
    return std::shared_ptr<regfile>(openRegFiles[coord].get(), [this](regfile* ptr) {
        ptr->inUse = false;
        regFilesCv.notify_one();
    });
}

void WorldFiles::closeRegFile(glm::ivec3 coord) {
    openRegFiles.erase(coord);
    regFilesCv.notify_one();
}

// Marks regfile as used and unmarks when shared_ptr dies
std::shared_ptr<regfile> WorldFiles::getRegFile(glm::ivec3 coord) {
    {
        std::lock_guard lock(regFilesMutex);
        const auto found = openRegFiles.find(coord);
        if (found != openRegFiles.end()) {
            if (found->second->inUse) {
                throw std::runtime_error("regfile is currently in use");
            }
            found->second->inUse = true;
            return useRegFile(found->first);
        }
    }
    return createRegFile(coord);
}

std::shared_ptr<regfile> WorldFiles::createRegFile(glm::ivec3 coord) {
    fs::path file = layers[coord[2]].folder/getRegionFilename(coord[0], coord[1]);
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

std::unique_ptr<ubyte[]> WorldFiles::readChunkData(
    int x, 
    int z, 
    uint32_t& length, 
    regfile* rfile
){
    if (generatorTestMode)
        return nullptr;
        
    int regionX, regionZ, localX, localZ;
    calc_reg_coords(x, z, regionX, regionZ, localX, localZ);
    int chunkIndex = localZ * REGION_SIZE + localX;

    files::rafile& file = rfile->file;
    size_t file_size = file.length();
    size_t table_offset = file_size - REGION_CHUNKS_COUNT * 4;

    uint32_t offset;
    file.seekg(table_offset + chunkIndex * 4);
    file.read((char*)(&offset), 4);
    offset = dataio::read_int32_big((const ubyte*)(&offset), 0);
    if (offset == 0){
        return nullptr;
    }

    file.seekg(offset);
    file.read((char*)(&offset), 4);
    length = dataio::read_int32_big((const ubyte*)(&offset), 0);
    auto data = std::make_unique<ubyte[]>(length);
    file.read((char*)data.get(), length);
    return data;
}

/// @brief Read missing chunks data (null pointers) from region file 
void WorldFiles::fetchChunks(WorldRegion* region, int x, int z, regfile* file) {
    ubyte** chunks = region->getChunks();
    uint32_t* sizes = region->getSizes();

    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        int chunk_x = (i % REGION_SIZE) + x * REGION_SIZE;
        int chunk_z = (i / REGION_SIZE) + z * REGION_SIZE;
        if (chunks[i] == nullptr) {
            chunks[i] = readChunkData(chunk_x, chunk_z, sizes[i], file).release();
        }
    }
}

void WorldFiles::writeRegion(int x, int z, int layer, WorldRegion* entry){
    fs::path filename = layers[layer].folder/getRegionFilename(x, z);

    glm::ivec3 regcoord(x, z, layer);
    if (auto regfile = getRegFile(regcoord)) {
        fetchChunks(entry, x, z, regfile.get());

        std::lock_guard lock(regFilesMutex);
        closeRegFile(regcoord);
    }
    
    char header[REGION_HEADER_SIZE] = REGION_FORMAT_MAGIC;
    header[8] = REGION_FORMAT_VERSION;
    header[9] = 0; // flags
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    file.write(header, REGION_HEADER_SIZE);

    size_t offset = REGION_HEADER_SIZE;
    char intbuf[4]{};
    uint offsets[REGION_CHUNKS_COUNT]{};
    
    ubyte** region = entry->getChunks();
    uint32_t* sizes = entry->getSizes();
    
    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        ubyte* chunk = region[i];
        if (chunk == nullptr){
            offsets[i] = 0;
        } else {
            offsets[i] = offset;

            size_t compressedSize = sizes[i];
            dataio::write_int32_big(compressedSize, (ubyte*)intbuf, 0);
            offset += 4 + compressedSize;

            file.write(intbuf, 4);
            file.write((const char*)chunk, compressedSize);
        }
    }
    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        dataio::write_int32_big(offsets[i], (ubyte*)intbuf, 0);
        file.write(intbuf, 4);
    }
}

void WorldFiles::writeRegions(int layer) {
    for (auto& it : layers[layer].regions){
        WorldRegion* region = it.second.get();
        if (region->getChunks() == nullptr || !region->isUnsaved())
            continue;
        glm::ivec2 key = it.first;
        writeRegion(key[0], key[1], layer, region);
    }
}

void WorldFiles::write(const World* world, const Content* content) {
    for (auto& layer : layers) {
        fs::create_directories(layer.folder);
    }
    if (world) {
        writeWorldInfo(world);
        if (!fs::exists(getPacksFile())) {
            writePacks(world->getPacks());
        }
    }
    if (generatorTestMode) {
        return;
    }
    
    writeIndices(content->getIndices());
    for (auto& layer : layers) {
        writeRegions(layer.layer);
    }
}

void WorldFiles::writePacks(const std::vector<ContentPack>& packs) {
    auto packsFile = getPacksFile();
    std::stringstream ss;
    ss << "# autogenerated; do not modify\n";
    for (const auto& pack : packs) {
        ss << pack.id << "\n";
    }
    files::write_string(packsFile, ss.str());
}

void WorldFiles::writeIndices(const ContentIndices* indices) {
    dynamic::Map root;
    uint count;
    auto& blocks = root.putList("blocks");
    count = indices->countBlockDefs();
    for (uint i = 0; i < count; i++) {
        const Block* def = indices->getBlockDef(i);
        blocks.put(def->name);
    }

    auto& items = root.putList("items");
    count = indices->countItemDefs();
    for (uint i = 0; i < count; i++) {
        const ItemDef* def = indices->getItemDef(i);
        items.put(def->name);
    }

    files::write_json(getIndicesFile(), &root);
}

void WorldFiles::writeWorldInfo(const World* world) {
    files::write_json(getWorldFile(), world->serialize().get());
}

bool WorldFiles::readWorldInfo(World* world) {
    fs::path file = getWorldFile();
    if (!fs::is_regular_file(file)) {
        std::cerr << "warning: world.json does not exists" << std::endl;
        return false;
    }

    auto root = files::read_json(file);
    world->deserialize(root.get());
    return true;
}

static void erase_pack_indices(dynamic::Map* root, const std::string& id) {
    auto prefix = id+":";
    auto blocks = root->list("blocks");
    for (uint i = 0; i < blocks->size(); i++) {
        auto name = blocks->str(i);
        if (name.find(prefix) != 0)
            continue;
        auto value = blocks->getValueWriteable(i);
        value->value = CORE_AIR;
    }

    auto items = root->list("items");
    for (uint i = 0; i < items->size(); i++) {
        auto name = items->str(i);
        if (name.find(prefix) != 0)
            continue;
        auto value = items->getValueWriteable(i);
        value->value = CORE_EMPTY;
    }
}

void WorldFiles::removeIndices(const std::vector<std::string>& packs) {
    auto root = files::read_json(getIndicesFile());
    for (const auto& id : packs) {
        erase_pack_indices(root.get(), id);
    }
    files::write_json(getIndicesFile(), root.get());
}

void WorldFiles::processRegionVoxels(int x, int z, regionproc func) {
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
            std::unique_ptr<ubyte[]> data (readChunkData(gx, gz, length, regfile.get()));
            if (data == nullptr)
                continue;
            if (func(data.get())) {
                put(gx, gz, REGION_LAYER_VOXELS, std::move(data), CHUNK_DATA_LEN, true);
            }
        }
    }
}

fs::path WorldFiles::getFolder() const {
    return directory;
}

fs::path WorldFiles::getRegionsFolder(int layer) const {
    return layers[layer].folder;
}
