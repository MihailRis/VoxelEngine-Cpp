#include "WorldFiles.h"

#include "rle.h"
#include "binary_io.h"
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

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <iostream>

const int SECTION_POSITION = 1;
const int SECTION_ROTATION = 2;
const int SECTION_FLAGS = 3;
const int PLAYER_FLAG_FLIGHT = 0x1;
const int PLAYER_FLAG_NOCLIP = 0x2;

const int WORLD_SECTION_MAIN = 1;
const int WORLD_SECTION_DAYNIGHT = 2;

namespace fs = std::filesystem;

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

WorldFiles::WorldFiles(fs::path directory, const DebugSettings& settings) 
	: directory(directory), 
	  generatorTestMode(settings.generatorTestMode),
	  doWriteLights(settings.doWriteLights) {
	compressionBuffer = new ubyte[CHUNK_DATA_LEN * 2];
}

WorldFiles::~WorldFiles(){
	delete[] compressionBuffer;
	for (auto it : regions){
	    delete it.second;
	}
	regions.clear();
}

WorldRegion* WorldFiles::getRegion(regionsmap& regions, int x, int z) {
	auto found = regions.find(ivec2(x, z));
	if (found == regions.end())
		return nullptr;
	return found->second;
}

WorldRegion* WorldFiles::getOrCreateRegion(regionsmap& regions, int x, int z) {
	WorldRegion* region = getRegion(regions, x, z);
	if (region == nullptr) {
		region = new WorldRegion();
		regions[ivec2(x, z)] = region;
	}
	return region;
}

ubyte* WorldFiles::compress(const ubyte* src, size_t srclen, size_t& len) {
	len = extrle::encode(src, srclen, compressionBuffer);
	ubyte* data = new ubyte[len];
	for (size_t i = 0; i < len; i++) {
		data[i] = compressionBuffer[i];
	}
	return data;
}

ubyte* WorldFiles::decompress(const ubyte* src, size_t srclen, size_t dstlen) {
	ubyte* decompressed = new ubyte[dstlen];
	extrle::decode(src, srclen, decompressed);
	return decompressed;
}

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

fs::path WorldFiles::getRegionFilename(int x, int y) const {
	std::string filename = std::to_string(x) + "_" + std::to_string(y) + ".bin";
	return fs::path(filename);
}

bool WorldFiles::parseRegionFilename(const std::string& name, int& x, int& y) {
    size_t sep = name.find('_');
    if (sep == std::string::npos || sep == 0 || sep == name.length()-1)
        return false;
    try {
        x = std::stoi(name.substr(0, sep));
        y = std::stoi(name.substr(sep+1));
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
	return directory/fs::path("world.json");
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

light_t* WorldFiles::getLights(int x, int z) {
	ubyte* data = getData(lights, getLightsFolder(), x, z, REGION_LAYER_LIGHTS);
	if (data == nullptr)
		return nullptr;
	return Lightmap::decode(data);
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

files::rafile* WorldFiles::getRegFile(glm::ivec3 coord, const fs::path& folder) {
    const auto found = openRegFiles.find(coord);
    if (found != openRegFiles.end()) {
        return found->second.get();
    }
    if (openRegFiles.size() == MAX_OPEN_REGION_FILES) {
        // [todo] replace with closing the most unused region
        auto iter = std::next(openRegFiles.begin(), rand() % openRegFiles.size());
        openRegFiles.erase(iter);
    }
    fs::path filename = folder/getRegionFilename(coord.x, coord.y);
    if (!fs::is_regular_file(filename)) {
        return nullptr;
    }
    openRegFiles[coord] = std::make_unique<files::rafile>(filename);
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
    files::rafile* file = WorldFiles::getRegFile(coord, folder);
    if (file == nullptr) {
        return nullptr;
    }

	size_t file_size = file->length();
	size_t table_offset = file_size - REGION_CHUNKS_COUNT * 4;

	uint32_t offset;
	file->seekg(table_offset + chunkIndex * 4);
	file->read((char*)(&offset), 4);
	offset = dataio::read_int32_big((const ubyte*)(&offset), 0);
	if (offset == 0){
		return nullptr;
	}
	file->seekg(offset);
	file->read((char*)(&offset), 4);
	length = dataio::read_int32_big((const ubyte*)(&offset), 0);
	ubyte* data = new ubyte[length];
	file->read((char*)data, length);
	if (data == nullptr) {
		std::cerr << "ERROR: failed to read data of chunk x("<< x <<"), z("<< z <<")" << std::endl;
	}
	return data;
}

void WorldFiles::fetchChunks(WorldRegion* region, int x, int y, fs::path folder, int layer) {
    ubyte** chunks = region->getChunks();
	uint32_t* sizes = region->getSizes();

    for (size_t i = 0; i < REGION_CHUNKS_COUNT; i++) {
        int chunk_x = (i % REGION_SIZE) + x * REGION_SIZE;
        int chunk_z = (i / REGION_SIZE) + y * REGION_SIZE;
        if (chunks[i] == nullptr) {
            chunks[i] = readChunkData(chunk_x, chunk_z, sizes[i], folder, layer);
        }
    }
}

void WorldFiles::writeRegion(int x, int y, WorldRegion* entry, fs::path folder, int layer){
    fs::path filename = folder/getRegionFilename(x, y);

    glm::ivec3 regcoord(x, y, layer);
    if (getRegFile(regcoord, folder)) {
        fetchChunks(entry, x, y, folder, layer);
        openRegFiles.erase(regcoord);
    }
    
	char header[10] = REGION_FORMAT_MAGIC;
	header[8] = REGION_FORMAT_VERSION;
	header[9] = 0; // flags
	std::ofstream file(filename, std::ios::out | std::ios::binary);
	file.write(header, 10);

	size_t offset = 10;
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
	for (auto it : regions){
		WorldRegion* region = it.second;
		if (region->getChunks() == nullptr || !region->isUnsaved())
			continue;
		ivec2 key = it.first;
		writeRegion(key.x, key.y, region, folder, layer);
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
		
	writeIndices(content->indices);
	writeRegions(regions, regionsFolder, REGION_LAYER_VOXELS);
	writeRegions(lights, lightsFolder, REGION_LAYER_LIGHTS);
}

void WorldFiles::writePacks(const World* world) {
	const auto& packs = world->getPacks();
	std::stringstream ss;
	ss << "# autogenerated; do not modify\n";
	for (const auto& pack : packs) {
		ss << pack.id << "\n";
	}
	files::write_string(getPacksFile(), ss.str());
}

void WorldFiles::writeIndices(const ContentIndices* indices) {
	json::JObject root;
	json::JArray& blocks = root.putArray("blocks");
	uint count = indices->countBlockDefs();
	for (uint i = 0; i < count; i++) {
		const Block* def = indices->getBlockDef(i);
		blocks.put(def->name);
	}
	files::write_string(getIndicesFile(), json::stringify(&root, true, "  "));
}

void WorldFiles::writeWorldInfo(const World* world) {
	json::JObject root;

	json::JObject& versionobj = root.putObj("version");
	versionobj.put("major", ENGINE_VERSION_MAJOR);
	versionobj.put("minor", ENGINE_VERSION_MINOR);

	root.put("name", world->name);
	root.put("seed", world->seed);
	
	json::JObject& timeobj = root.putObj("time");
	timeobj.put("day-time", world->daytime);
	timeobj.put("day-time-speed", world->daytimeSpeed);

	files::write_string(getWorldFile(), json::stringify(&root, true, "  "));
}

bool WorldFiles::readWorldInfo(World* world) {
	fs::path file = getWorldFile();
	if (!fs::is_regular_file(file)) {
		std::cerr << "warning: world.json does not exists" << std::endl;
		return false;
	}

	std::unique_ptr<json::JObject> root(files::read_json(file));
	root->str("name", world->name);
	root->num("seed", world->seed);

	json::JObject* verobj = root->obj("version");
	if (verobj) {
		int major=0, minor=-1;
		verobj->num("major", major);
		verobj->num("minor", minor);
		std::cout << "world version: " << major << "." << minor << std::endl;
	}

	json::JObject* timeobj = root->obj("time");
	if (timeobj) {
		timeobj->num("day-time", world->daytime);
		timeobj->num("day-time-speed", world->daytimeSpeed);
	}

	return true;
}

void WorldFiles::writePlayer(Player* player){
	vec3 position = player->hitbox->position;
	json::JObject root;
	json::JArray& posarr = root.putArray("position");
	posarr.put(position.x);
	posarr.put(position.y);
	posarr.put(position.z);

	json::JArray& rotarr = root.putArray("rotation");
	rotarr.put(player->camX);
	rotarr.put(player->camY);
	
	root.put("flight", player->flight);
	root.put("noclip", player->noclip);

	files::write_string(getPlayerFile(), json::stringify(&root, true, "  "));
}

bool WorldFiles::readPlayer(Player* player) {
	fs::path file = getPlayerFile();
	if (!fs::is_regular_file(file)) {
		std::cerr << "warning: player.json does not exists" << std::endl;
		return false;
	}

	std::unique_ptr<json::JObject> root(files::read_json(file));
	json::JArray* posarr = root->arr("position");
	vec3& position = player->hitbox->position;
	position.x = posarr->num(0);
	position.y = posarr->num(1);
	position.z = posarr->num(2);
	player->camera->position = position;

	json::JArray* rotarr = root->arr("rotation");
	player->camX = rotarr->num(0);
	player->camY = rotarr->num(1);

	root->flag("flight", player->flight);
	root->flag("noclip", player->noclip);
	return true;
}
