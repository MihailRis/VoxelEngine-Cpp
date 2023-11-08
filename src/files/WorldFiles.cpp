#include "WorldFiles.h"

#include "files.h"
#include "rle.h"
#include "../window/Camera.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../voxels/voxel.h"
#include "../voxels/Chunk.h"
#include "../typedefs.h"
#include "../maths/voxmaths.h"

#include <cassert>
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

using glm::ivec2;
using glm::vec3;
using std::ios;
using std::unique_ptr;

int bytes2Int(const ubyte* src, size_t offset){
	return (src[offset] << 24) | (src[offset+1] << 16) | (src[offset+2] << 8) | (src[offset+3]);
}

void int2Bytes(int value, ubyte* dest, size_t offset){
	dest[offset] = (char) (value >> 24 & 255);
	dest[offset+1] = (char) (value >> 16 & 255);
	dest[offset+2] = (char) (value >> 8 & 255);
	dest[offset+3] = (char) (value >> 0 & 255);
}

void floatToBytes(float fvalue, ubyte* dest, size_t offset){
	uint32_t value = *((uint32_t*)&fvalue);
	dest[offset] = (char) (value >> 24 & 255);
	dest[offset+1] = (char) (value >> 16 & 255);
	dest[offset+2] = (char) (value >> 8 & 255);
	dest[offset+3] = (char) (value >> 0 & 255);
}

float bytes2Float(ubyte* src, uint offset){
	uint32_t value = ((src[offset] << 24) |
					  (src[offset+1] << 16) |
					  (src[offset+2] << 8) |
					  (src[offset+3]));
	return *(float*)(&value);
}

WorldFiles::WorldFiles(std::string directory, size_t mainBufferCapacity) : directory(directory){
	mainBufferIn = new ubyte[CHUNK_DATA_LEN * 2];
	compressionBuffer = new ubyte[CHUNK_DATA_LEN * 2];
}

WorldFiles::~WorldFiles(){
	delete[] mainBufferIn;
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

std::string WorldFiles::getRegionFile(int x, int y) {
	return directory + std::to_string(x) + "_" + std::to_string(y) + ".bin";
}

std::string WorldFiles::getPlayerFile() {
	return directory + "/player.bin";
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
	int regionX = floordiv(x, REGION_SIZE);
	int regionY = floordiv(y, REGION_SIZE);

	int localX = x - (regionX * REGION_SIZE);
	int localY = y - (regionY * REGION_SIZE);

	int chunkIndex = localY * REGION_SIZE + localX;

	std::string filename = getRegionFile(regionX, regionY);
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

void WorldFiles::write(){
	for (auto it = regions.begin(); it != regions.end(); it++){
		if (it->second.chunksData == nullptr || !it->second.unsaved)
			continue;
		ivec2 key = it->first;
		writeRegion(key.x, key.y, it->second);
	}
}

void WorldFiles::writePlayer(Player* player){
	ubyte dst[1+3*4 + 1+2*4 + 1+1];

	vec3 position = player->hitbox->position;

	size_t offset = 0;
	dst[offset++] = SECTION_POSITION;
	floatToBytes(position.x, dst, offset); offset += 4;
	floatToBytes(position.y, dst, offset); offset += 4;
	floatToBytes(position.z, dst, offset); offset += 4;

	dst[offset++] = SECTION_ROTATION;
	floatToBytes(player->camX, dst, offset); offset += 4;
	floatToBytes(player->camY, dst, offset); offset += 4;

	dst[offset++] = SECTION_FLAGS;
	dst[offset++] = player->flight * PLAYER_FLAG_FLIGHT |
					player->noclip * PLAYER_FLAG_NOCLIP;

	files::write_binary_file(getPlayerFile(), (const char*)dst, sizeof(dst));
}

bool WorldFiles::readPlayer(Player* player) {
	size_t length = 0;
	ubyte* data = (ubyte*)files::read_binary_file(getPlayerFile(), length);
	if (data == nullptr){
		std::cerr << "could not to read player.bin (ignored)" << std::endl;
		return false;
	}
	vec3 position = player->hitbox->position;
	size_t offset = 0;
	while (offset < length){
		char section = data[offset++];
		switch (section){
		case SECTION_POSITION:
			position.x = bytes2Float(data, offset); offset += 4;
			position.y = bytes2Float(data, offset); offset += 4;
			position.z = bytes2Float(data, offset); offset += 4;
			break;
		case SECTION_ROTATION:
			player->camX = bytes2Float(data, offset); offset += 4;
			player->camY = bytes2Float(data, offset); offset += 4;
			break;
		case SECTION_FLAGS:
			{
				ubyte flags = data[offset++];
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

	char header[10] = ".VOXREG";
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
