#include "WorldFiles.h"

#include "rle.h"
#include "../window/Camera.h"
#include "../content/Content.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../voxels/voxel.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../typedefs.h"
#include "../maths/voxmaths.h"
#include "../world/World.h"
#include "../lighting/Lightmap.h"

#include "../util/data_io.h"
#include "../coders/json.h"
#include "../constants.h"
#include "../items/ItemDef.h"
#include "../items/Inventory.h"

#include "../data/dynamic.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cstring>

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
            "region format "+std::to_string(version)+" is not supported");
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

const char* WorldFiles::WORLD_FILE = "world.json";

WorldFiles::WorldFiles(fs::path directory, const DebugSettings& settings) 
	: directory(directory), 
	  generatorTestMode(settings.generatorTestMode),
	  doWriteLights(settings.doWriteLights) {
	compressionBuffer.reset(new ubyte[CHUNK_DATA_LEN * 2]);
}

WorldFiles::~WorldFiles(){
}

WorldRegion* WorldFiles::getRegion(regionsmap& regions, int x, int z) {
	auto found = regions.find(glm::ivec2(x, z));
	if (found == regions.end())
		return nullptr;
	return found->second.get();
}

WorldRegion* WorldFiles::getOrCreateRegion(regionsmap& regions, int x, int z) {
	WorldRegion* region = getRegion(regions, x, z);
	if (region == nullptr) {
		region = new WorldRegion();
		regions[glm::ivec2(x, z)].reset(region);
	}
	return region;
}

ubyte* WorldFiles::compress(const ubyte* src, size_t srclen, size_t& len) {
    ubyte* buffer = this->compressionBuffer.get();
    
	len = extrle::encode(src, srclen, buffer);
	ubyte* data = new ubyte[len];
	for (size_t i = 0; i < len; i++) {
		data[i] = buffer[i];
	}
	return data;
}

ubyte* WorldFiles::decompress(const ubyte* src, size_t srclen, size_t dstlen) {
	ubyte* decompressed = new ubyte[dstlen];
	extrle::decode(src, srclen, decompressed);
	return decompressed;
}

int WorldFiles::getVoxelRegionVersion(int x, int z) {
    regfile* rf = getRegFile(glm::ivec3(x, z, REGION_LAYER_VOXELS), getRegionsFolder());
    if (rf == nullptr) {
        return 0;
    }
    return rf->version;
}

int WorldFiles::getVoxelRegionsVersion() {
    fs::path regionsFolder = getRegionsFolder();
    if (!fs::is_directory(regionsFolder)) {
        return REGION_FORMAT_VERSION;
    }
    for (auto file : fs::directory_iterator(regionsFolder)) {
        int x;
        int z;
        if (!parseRegionFilename(file.path().stem().string(), x, z)) {
            continue;
        }
        regfile* rf = getRegFile(glm::ivec3(x, z, REGION_LAYER_VOXELS), regionsFolder);
        return rf->version;
    }
    return REGION_FORMAT_VERSION;
}

/* 
 * Compress and store chunk voxels data in region 
 * @param x chunk.x
 * @param z chunk.z
 */
void WorldFiles::put(int x, int z, const ubyte* voxelData) {
    int regionX = floordiv(x, REGION_SIZE);
	int regionZ = floordiv(z, REGION_SIZE);
	int localX = x - (regionX * REGION_SIZE);
	int localZ = z - (regionZ * REGION_SIZE);

	/* Writing Voxels */ {
		WorldRegion* region = getOrCreateRegion(regions, regionX, regionZ);
		region->setUnsaved(true);
		size_t compressedSize;
		ubyte* data = compress(voxelData, CHUNK_DATA_LEN, compressedSize);
		region->put(localX, localZ, data, compressedSize);
	}
}

/*
 * Store chunk (voxels and lights) in region (existing or new)
 */
void WorldFiles::put(Chunk* chunk){
	assert(chunk != nullptr);

	int regionX = floordiv(chunk->x, REGION_SIZE);
	int regionZ = floordiv(chunk->z, REGION_SIZE);
	int localX = chunk->x - (regionX * REGION_SIZE);
	int localZ = chunk->z - (regionZ * REGION_SIZE);

	/* Writing Voxels */ {
        size_t compressedSize;
        std::unique_ptr<ubyte[]> chunk_data (chunk->encode());
		ubyte* data = compress(chunk_data.get(), CHUNK_DATA_LEN, compressedSize);

		WorldRegion* region = getOrCreateRegion(regions, regionX, regionZ);
		region->setUnsaved(true);
		region->put(localX, localZ, data, compressedSize);
	}
	if (doWriteLights && chunk->isLighted()) {
        size_t compressedSize;
        std::unique_ptr<ubyte[]> light_data (chunk->lightmap->encode());
		ubyte* data = compress(light_data.get(), LIGHTMAP_DATA_LEN, compressedSize);

		WorldRegion* region = getOrCreateRegion(lights, regionX, regionZ);
		region->setUnsaved(true);
		region->put(localX, localZ, data, compressedSize);
	}
}

fs::path WorldFiles::getRegionsFolder() const {
	return directory/fs::path("regions");
}

fs::path WorldFiles::getLightsFolder() const {
	return directory/fs::path("lights");
}

fs::path WorldFiles::getRegionFilename(int x, int z) const {
	return fs::path(std::to_string(x) + "_" + std::to_string(z) + ".bin");
}

/* 
 * Extract X and Z from 'X_Z.bin' region file name.
 * @param name source region file name
 * @param x parsed X destination
 * @param z parsed Z destination
 * @return false if std::invalid_argument or std::out_of_range occurred
 */
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

ubyte* WorldFiles::getChunk(int x, int z){
	return getData(regions, getRegionsFolder(), x, z, REGION_LAYER_VOXELS);
}

/* Get cached lights for chunk at x,z 
 * @return lights data or nullptr */
light_t* WorldFiles::getLights(int x, int z) {
	std::unique_ptr<ubyte> data (getData(lights, getLightsFolder(), x, z, REGION_LAYER_LIGHTS));
	if (data == nullptr)
		return nullptr;
	return Lightmap::decode(data.get());
}

ubyte* WorldFiles::getData(regionsmap& regions, const fs::path& folder, 
                           int x, int z, int layer) {
	int regionX = floordiv(x, REGION_SIZE);
	int regionZ = floordiv(z, REGION_SIZE);

	int localX = x - (regionX * REGION_SIZE);
	int localZ = z - (regionZ * REGION_SIZE);

	WorldRegion* region = getOrCreateRegion(regions, regionX, regionZ);
	ubyte* data = region->getChunkData(localX, localZ);
	if (data == nullptr) {
		uint32_t size;
		data = readChunkData(x, z, size, folder, layer);
		if (data != nullptr) {
			region->put(localX, localZ, data, size);
		}
	}
	if (data != nullptr) {
        size_t size = region->getChunkDataSize(localX, localZ);
		return decompress(data, size, CHUNK_DATA_LEN);
	}
	return nullptr;
}


regfile* WorldFiles::getRegFile(glm::ivec3 coord, const fs::path& folder) {
    const auto found = openRegFiles.find(coord);
    if (found != openRegFiles.end()) {
        return found->second.get();
    }
    if (openRegFiles.size() == MAX_OPEN_REGION_FILES) {
        // [todo] replace with closing the most unused region
        auto iter = std::next(openRegFiles.begin(), rand() % openRegFiles.size());
        openRegFiles.erase(iter);
    }
	fs::path filename = folder / getRegionFilename(coord[0], coord[1]);
    if (!fs::is_regular_file(filename)) {
        return nullptr;
    }
    openRegFiles[coord] = std::make_unique<regfile>(filename);
    return openRegFiles[coord].get();
}

ubyte* WorldFiles::readChunkData(int x, 
                                 int z, 
                                 uint32_t& length, 
                                 fs::path folder, 
                                 int layer){
	if (generatorTestMode)
		return nullptr;
		
	int regionX = floordiv(x, REGION_SIZE);
	int regionZ = floordiv(z, REGION_SIZE);
	int localX = x - (regionX * REGION_SIZE);
	int localZ = z - (regionZ * REGION_SIZE);
	int chunkIndex = localZ * REGION_SIZE + localX;
 
    glm::ivec3 coord(regionX, regionZ, layer);
    regfile* rfile = WorldFiles::getRegFile(coord, folder);
    if (rfile == nullptr) {
        return nullptr;
    }
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
	ubyte* data = new ubyte[length];
	file.read((char*)data, length);
	if (data == nullptr) {
		std::cerr << "ERROR: failed to read data of chunk x("<< x <<"), z("<< z <<")" << std::endl;
	}
	return data;
}

/* Read missing chunks data (null pointers) from region file 
 * @param layer used as third part of openRegFiles map key 
 * (see REGION_LAYER_* constants)
 */
void WorldFiles::fetchChunks(WorldRegion* region, int x, int z, fs::path folder, int layer) {
    ubyte** chunks = region->getChunks();
	uint32_t* sizes = region->getSizes();

    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        int chunk_x = (i % REGION_SIZE) + x * REGION_SIZE;
        int chunk_z = (i / REGION_SIZE) + z * REGION_SIZE;
        if (chunks[i] == nullptr) {
            chunks[i] = readChunkData(chunk_x, chunk_z, sizes[i], folder, layer);
        }
    }
}

/* Write or rewrite region file
 * @param x region X
 * @param z region Z
 * @param layer used as third part of openRegFiles map key 
 * (see REGION_LAYER_* constants)
 */
void WorldFiles::writeRegion(int x, int z, WorldRegion* entry, fs::path folder, int layer){
    fs::path filename = folder/getRegionFilename(x, z);

    glm::ivec3 regcoord(x, z, layer);
    if (getRegFile(regcoord, folder)) {
        fetchChunks(entry, x, z, folder, layer);
        openRegFiles.erase(regcoord);
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

void WorldFiles::writeRegions(regionsmap& regions, const fs::path& folder, int layer) {
	for (auto& it : regions){
		WorldRegion* region = it.second.get();
		if (region->getChunks() == nullptr || !region->isUnsaved())
			continue;
		glm::ivec2 key = it.first;
		writeRegion(key[0], key[1], region, folder, layer);
	}
}

void WorldFiles::write(const World* world, const Content* content) {
	fs::path regionsFolder = getRegionsFolder();
	fs::path lightsFolder = getLightsFolder();

	fs::create_directories(regionsFolder);
	fs::create_directories(lightsFolder);

    if (world) {
	    writeWorldInfo(world);
		writePacks(world);
	}
	if (generatorTestMode)
		return;
		
	writeIndices(content->getIndices());
	writeRegions(regions, regionsFolder, REGION_LAYER_VOXELS);
	writeRegions(lights, lightsFolder, REGION_LAYER_LIGHTS);
}

void WorldFiles::writePacks(const World* world) {
    auto packsFile = getPacksFile();
    if (fs::is_regular_file(packsFile)) {
        return;
    }

	const auto& packs = world->getPacks();
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
	dynamic::Map root;

	auto& versionobj = root.putMap("version");
	versionobj.put("major", ENGINE_VERSION_MAJOR);
	versionobj.put("minor", ENGINE_VERSION_MINOR);

	root.put("name", world->getName());
	root.put("seed", world->getSeed());
	
    auto& timeobj = root.putMap("time");
	timeobj.put("day-time", world->daytime);
	timeobj.put("day-time-speed", world->daytimeSpeed);
    timeobj.put("total-time", world->totalTime);

	files::write_json(getWorldFile(), &root);
}

bool WorldFiles::readWorldInfo(World* world) {
	fs::path file = getWorldFile();
	if (!fs::is_regular_file(file)) {
		std::cerr << "warning: world.json does not exists" << std::endl;
		return false;
	}

	auto root = files::read_json(file);
    
    world->setName(root->getStr("name", world->getName()));
    world->setSeed(root->getInt("seed", world->getSeed()));

	auto verobj = root->map("version");
	if (verobj) {
		int major=0, minor=-1;
		verobj->num("major", major);
		verobj->num("minor", minor);
		std::cout << "world version: " << major << "." << minor << std::endl;
	}

	auto timeobj = root->map("time");
	if (timeobj) {
		timeobj->num("day-time", world->daytime);
		timeobj->num("day-time-speed", world->daytimeSpeed);
        timeobj->num("total-time", world->totalTime);
	}

	return true;
}

void WorldFiles::writePlayer(Player* player) {
	files::write_json(getPlayerFile(), player->write().release());
}

bool WorldFiles::readPlayer(Player* player) {
	fs::path file = getPlayerFile();
	if (!fs::is_regular_file(file)) {
		std::cerr << "warning: player.json does not exists" << std::endl;
		return false;
	}

	auto root = files::read_json(file);
	auto posarr = root->list("position");
	glm::vec3& position = player->hitbox->position;
	position.x = posarr->num(0);
	position.y = posarr->num(1);
	position.z = posarr->num(2);
	player->camera->position = position;

	auto rotarr = root->list("rotation");
	player->cam.x = rotarr->num(0);
	player->cam.y = rotarr->num(1);

	if (root->has("spawnpoint")) {
		auto sparr = root->list("spawnpoint");
		player->setSpawnPoint(glm::vec3(
			sparr->num(0),
			sparr->num(1),
			sparr->num(2)
		));
	} else {
		player->setSpawnPoint(position);
	}

	root->flag("flight", player->flight);
	root->flag("noclip", player->noclip);
    player->setChosenSlot(root->getInt("chosen-slot", player->getChosenSlot()));

    auto invmap = root->map("inventory");
    if (invmap) {
        player->getInventory()->read(invmap);
    }
	return true;
}

void WorldFiles::addPack(const std::string& id) {
    auto packs = files::read_list(getPacksFile());
    packs.push_back(id);

	std::stringstream ss;
	ss << "# autogenerated; do not modify\n";
	for (const auto& pack : packs) {
		ss << pack << "\n";
	}
	files::write_string(getPacksFile(), ss.str());
}
