#include "WorldFiles.h"

#include "files.h"
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

#include "../coders/json.h"
#include "../constants.h"

#include <cassert>
#include <string>
#include <iostream>
#include <cstdint>
#include <memory>
#include <fstream>
#include <iostream>

#define SECTION_POSITION 1
#define SECTION_ROTATION 2
#define SECTION_FLAGS 3
#define PLAYER_FLAG_FLIGHT 0x1
#define PLAYER_FLAG_NOCLIP 0x2

#define WORLD_SECTION_MAIN 1
#define WORLD_SECTION_DAYNIGHT 2

using glm::ivec2;
using glm::vec3;
using std::ios;
using std::string;
using std::unique_ptr;
using std::filesystem::path;
namespace fs = std::filesystem;

int bytes2Int(const ubyte* src, size_t offset){
	return (src[offset] << 24) | (src[offset+1] << 16) | (src[offset+2] << 8) | (src[offset+3]);
}

void int2Bytes(int value, ubyte* dest, size_t offset){
	dest[offset] = (char) (value >> 24 & 255);
	dest[offset+1] = (char) (value >> 16 & 255);
	dest[offset+2] = (char) (value >> 8 & 255);
	dest[offset+3] = (char) (value >> 0 & 255);
}

WorldFiles::WorldFiles(path directory, bool generatorTestMode) 
	: directory(directory), generatorTestMode(generatorTestMode) {
	compressionBuffer = new ubyte[CHUNK_DATA_LEN * 2];
}

WorldFiles::~WorldFiles(){
	delete[] compressionBuffer;
	for (auto it = regions.begin(); it != regions.end(); it++){
	    WorldRegion region = it->second;
	    if (region.chunksData == nullptr)
	    	continue;
	    for (size_t i = 0; i < REGION_VOL; i++){
	    	delete[] region.chunksData[i];
	    }
	    delete[] region.chunksData;
	}
	regions.clear();
}

void WorldFiles::put(Chunk* chunk){
	assert(chunk != nullptr);

	int regionX = floordiv(chunk->x, REGION_SIZE);
	int regionY = floordiv(chunk->z, REGION_SIZE);

	int localX = chunk->x - (regionX * REGION_SIZE);
	int localY = chunk->z - (regionY * REGION_SIZE);

	ivec2 key(regionX, regionY);

	auto found = regions.find(key);
	if (found == regions.end()) {
		ubyte** chunksData = new ubyte*[REGION_VOL];
		uint32_t* compressedSizes = new uint32_t[REGION_VOL];
		for (uint i = 0; i < REGION_VOL; i++) {
			chunksData[i] = nullptr;
		}
		regions[key] = { chunksData, compressedSizes, true };
	}

	WorldRegion& region = regions[key];
	region.unsaved = true;
	size_t chunk_index = localY * REGION_SIZE + localX;
	ubyte* target_chunk = region.chunksData[chunk_index];
	if (target_chunk) {
		delete[] target_chunk;
	}

	ubyte* chunk_data = chunk->encode();
	size_t compressedSize = extrle::encode(chunk_data, CHUNK_DATA_LEN, compressionBuffer);
	delete[] chunk_data;
	ubyte* data = new ubyte[compressedSize];
	for (size_t i = 0; i < compressedSize; i++) {
		data[i] = compressionBuffer[i];
	}
	region.chunksData[chunk_index] = data;
	region.compressedSizes[chunk_index] = compressedSize;
}

path WorldFiles::getRegionsFolder() const {
	return directory/path("regions");
}

path WorldFiles::getRegionFile(int x, int y) const {
	return getRegionsFolder()/path(std::to_string(x) + "_" + std::to_string(y) + ".bin");
}

path WorldFiles::getPlayerFile() const {
	return directory/path("player.json");
}

path WorldFiles::getWorldFile() const {
	return directory/path("world.json");
}

path WorldFiles::getIndicesFile() const {
	return directory/path("indices.json");
}

path WorldFiles::getOldPlayerFile() const {
	return directory/path("player.bin");
}

path WorldFiles::getOldWorldFile() const {
	return directory/path("world.bin");
}

ubyte* WorldFiles::getChunk(int x, int y){
	int regionX = floordiv(x, REGION_SIZE);
	int regionY = floordiv(y, REGION_SIZE);

	int localX = x - (regionX * REGION_SIZE);
	int localY = y - (regionY * REGION_SIZE);

	int chunkIndex = localY * REGION_SIZE + localX;
	assert(chunkIndex >= 0 && chunkIndex < REGION_VOL);

	ivec2 key(regionX, regionY);

	auto found = regions.find(key);
	if (found == regions.end()) {
		ubyte** chunksData = new ubyte * [REGION_VOL];
		uint32_t* compressedSizes = new uint32_t[REGION_VOL];
		for (uint i = 0; i < REGION_VOL; i++) {
			chunksData[i] = nullptr;
		}
		regions[key] = { chunksData, compressedSizes, true };
	}

	WorldRegion& region = regions[key];
	ubyte* data = region.chunksData[chunkIndex];
	if (data == nullptr) {
		data = readChunkData(x, y, region.compressedSizes[chunkIndex]);
		if (data) {
			region.chunksData[chunkIndex] = data;
		}
	}
	if (data) {
		ubyte* decompressed = new ubyte[CHUNK_DATA_LEN];
		extrle::decode(data, region.compressedSizes[chunkIndex], decompressed);
		return decompressed;
	}
	return nullptr;
}

ubyte* WorldFiles::readChunkData(int x, int y, uint32_t& length){
	if (generatorTestMode)
		return nullptr;
		
	int regionX = floordiv(x, REGION_SIZE);
	int regionY = floordiv(y, REGION_SIZE);

	int localX = x - (regionX * REGION_SIZE);
	int localY = y - (regionY * REGION_SIZE);

	int chunkIndex = localY * REGION_SIZE + localX;

	path filename = getRegionFile(regionX, regionY);
	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()){
		return nullptr;
	}
	input.seekg(0, ios::end);
	size_t file_size = input.tellg();
	size_t table_offset = file_size - REGION_VOL * 4;

	uint32_t offset;
	input.seekg(table_offset + chunkIndex * 4);
	input.read((char*)(&offset), 4);
	offset = bytes2Int((const ubyte*)(&offset), 0);
	if (offset == 0){
		input.close();
		return nullptr;
	}
	input.seekg(offset);
	input.read((char*)(&offset), 4);
	length = bytes2Int((const ubyte*)(&offset), 0);
	ubyte* data = new ubyte[length];
	input.read((char*)data, length);
	input.close();
	return data;
}

void WorldFiles::write(const World* world, const Content* content) {
	path directory = getRegionsFolder();
	if (!std::filesystem::is_directory(directory)) {
		std::filesystem::create_directories(directory);
	}
	writeWorldInfo(world);
	if (generatorTestMode)
		return;
	writeIndices(content->indices);
	for (auto it = regions.begin(); it != regions.end(); it++){
		if (it->second.chunksData == nullptr || !it->second.unsaved)
			continue;
		ivec2 key = it->first;
		writeRegion(key.x, key.y, it->second);
	}
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

// TODO: remove in v0.16
bool WorldFiles::readOldWorldInfo(World* world) {
	size_t length = 0;
	ubyte* data = (ubyte*)files::read_bytes(getWorldFile(), length);
	assert(data != nullptr);
	BinaryReader inp(data, length);
	inp.checkMagic(WORLD_FORMAT_MAGIC, 8);
	/*ubyte version = */inp.get();
	while (inp.hasNext()) {
		ubyte section = inp.get();
		switch (section) {
		case WORLD_SECTION_MAIN:
			world->seed = inp.getInt64();
			world->name = inp.getString();
			break;
		case WORLD_SECTION_DAYNIGHT:
			world->daytime = inp.getFloat32();
			world->daytimeSpeed = inp.getFloat32();
			break;
		}
	}
	return false;
}
bool WorldFiles::readOldPlayer(Player* player) {
	size_t length = 0;
	ubyte* data = (ubyte*)files::read_bytes(getPlayerFile(), length);
	if (data == nullptr){
		std::cerr << "could not to read player.bin (ignored)" << std::endl;
		return false;
	}
	vec3 position = player->hitbox->position;
	BinaryReader inp(data, length);
	while (inp.hasNext()) {
		ubyte section = inp.get();
		switch (section) {
		case SECTION_POSITION:
			position.x = inp.getFloat32();
			position.y = inp.getFloat32();
			position.z = inp.getFloat32();
			break;
		case SECTION_ROTATION:
			player->camX = inp.getFloat32();
			player->camY = inp.getFloat32();
			break;
		case SECTION_FLAGS: 
			{
				ubyte flags = inp.get();
				player->flight = flags & PLAYER_FLAG_FLIGHT;
				player->noclip = flags & PLAYER_FLAG_NOCLIP;
			}
			break;
		}
	}

	player->hitbox->position = position;
	player->camera->position = position + vec3(0, 1, 0);
	return true;
}
// ----- // ----- //

bool WorldFiles::readWorldInfo(World* world) {
	path file = getWorldFile();
	if (!fs::is_regular_file(file)) {
		// TODO: remove in v0.16
		file = getOldWorldFile();
		if (fs::is_regular_file(file)) {
			readOldWorldInfo(world);
		}
		std::cerr << "warning: world.json does not exists" << std::endl;
		return false;
	}

	unique_ptr<json::JObject> root(files::read_json(file));
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
	path file = getPlayerFile();
	if (!fs::is_regular_file(file)) {
		// TODO: remove in v0.16
		file = getOldPlayerFile();
		if (fs::is_regular_file(file)) {
			readOldPlayer(player);
		}
		std::cerr << "warning: player.json does not exists" << std::endl;
		return false;
	}

	unique_ptr<json::JObject> root(files::read_json(file));
	json::JArray* posarr = root->arr("position");
	vec3& position = player->hitbox->position;
	position.x = posarr->num(0);
	position.y = posarr->num(1);
	position.z = posarr->num(2);

	json::JArray* rotarr = root->arr("rotation");
	player->camX = rotarr->num(0);
	player->camY = rotarr->num(1);

	root->flag("flight", player->flight);
	root->flag("noclip", player->noclip);
	return true;
}

void WorldFiles::writeRegion(int x, int y, WorldRegion& entry){
	ubyte** region = entry.chunksData;
	uint32_t* sizes = entry.compressedSizes;
	for (size_t i = 0; i < REGION_VOL; i++) {
		int chunk_x = (i % REGION_SIZE) + x * REGION_SIZE;
		int chunk_y = (i / REGION_SIZE) + y * REGION_SIZE;
		if (region[i] == nullptr) {
			region[i] = readChunkData(chunk_x, chunk_y, sizes[i]);
		}
	}

	char header[10] = REGION_FORMAT_MAGIC;
	header[8] = REGION_FORMAT_VERSION;
	header[9] = 0; // flags
	std::ofstream file(getRegionFile(x, y), ios::out | ios::binary);
	file.write(header, 10);

	size_t offset = 10;
	char intbuf[4]{};
	uint offsets[REGION_VOL]{};
	
	for (size_t i = 0; i < REGION_VOL; i++) {
		ubyte* chunk = region[i];
		if (chunk == nullptr){
			offsets[i] = 0;
		} else {
			offsets[i] = offset;

			size_t compressedSize = sizes[i];
			int2Bytes(compressedSize, (ubyte*)intbuf, 0);
			offset += 4 + compressedSize;

			file.write(intbuf, 4);
			file.write((const char*)chunk, compressedSize);
		}
	}
	for (size_t i = 0; i < REGION_VOL; i++) {
		int2Bytes(offsets[i], (ubyte*)intbuf, 0);
		file.write(intbuf, 4);
	}
}
