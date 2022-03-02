#include "WorldFiles.h"

#include "files.h"
#include "../window/Camera.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../voxels/Chunk.h"

union {
	long _key;
	int _coords[2];
} _tempcoords;

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <iostream>

#define SECTION_POSITION 1
#define SECTION_ROTATION 2

unsigned long WorldFiles::totalCompressed = 0;

int bytes2Int(const unsigned char* src, unsigned int offset){
	return (src[offset] << 24) | (src[offset+1] << 16) | (src[offset+2] << 8) | (src[offset+3]);
}

void int2Bytes(int value, char* dest, unsigned int offset){
	dest[offset] = (char) (value >> 24 & 255);
	dest[offset+1] = (char) (value >> 16 & 255);
	dest[offset+2] = (char) (value >> 8 & 255);
	dest[offset+3] = (char) (value >> 0 & 255);
}

void float2Bytes(float fvalue, char* dest, unsigned int offset){
	uint32_t value = *((uint32_t*)&fvalue);
	dest[offset] = (char) (value >> 24 & 255);
	dest[offset+1] = (char) (value >> 16 & 255);
	dest[offset+2] = (char) (value >> 8 & 255);
	dest[offset+3] = (char) (value >> 0 & 255);
}

float bytes2Float(char* srcs, unsigned int offset){
	unsigned char* src = (unsigned char*) srcs;
	uint32_t value = ((src[offset] << 24) |
					  (src[offset+1] << 16) |
					  (src[offset+2] << 8) |
					  (src[offset+3]));
	return *(float*)(&value);
}

WorldFiles::WorldFiles(const char* directory, size_t mainBufferCapacity) : directory(directory){
	mainBufferIn = new char[CHUNK_VOL*2];
	mainBufferOut = new char[mainBufferCapacity];
}

WorldFiles::~WorldFiles(){
	delete[] mainBufferIn;
	delete[] mainBufferOut;
	std::unordered_map<long, char**>::iterator it;
	for (it = regions.begin(); it != regions.end(); it++){
	    char** region = it->second;
	    if (region == nullptr)
	    	continue;
	    for (unsigned int i = 0; i < REGION_VOL; i++){
	    	delete[] region[i];
	    }
	    delete[] region;
	}
	regions.clear();
}

void WorldFiles::put(const char* chunkData, int x, int y){
	assert(chunkData != nullptr);

	int regionX = x >> REGION_SIZE_BIT;
	int regionY = y >> REGION_SIZE_BIT;

	int localX = x - (regionX << REGION_SIZE_BIT);
	int localY = y - (regionY << REGION_SIZE_BIT);

	_tempcoords._coords[0] = regionX;
	_tempcoords._coords[1] = regionY;
	char** region = regions[_tempcoords._key];
	if (region == nullptr){
		region = new char*[REGION_VOL];
		for (unsigned int i = 0; i < REGION_VOL; i++)
			region[i] = nullptr;
		regions[_tempcoords._key] = region;
	}
	char* targetChunk = region[localY * REGION_SIZE + localX];
	if (targetChunk == nullptr){
		targetChunk = new char[CHUNK_VOL];
		region[localY * REGION_SIZE + localX] = targetChunk;
		totalCompressed += CHUNK_VOL;
	}
	for (unsigned int i = 0; i < CHUNK_VOL; i++)
		targetChunk[i] = chunkData[i];

}

std::string WorldFiles::getRegionFile(int x, int y) {
	return directory + std::to_string(x) + "_" + std::to_string(y) + ".bin";
}

std::string WorldFiles::getPlayerFile() {
	return directory + "/player.bin";
}

bool WorldFiles::getChunk(int x, int y, char* out){
	assert(out != nullptr);

	int regionX = x >> REGION_SIZE_BIT;
	int regionY = y >> REGION_SIZE_BIT;

	int localX = x - (regionX << REGION_SIZE_BIT);
	int localY = y - (regionY << REGION_SIZE_BIT);
	int chunkIndex = localY * REGION_SIZE + localX;
	assert(chunkIndex >= 0 && chunkIndex < REGION_VOL);

	_tempcoords._coords[0] = regionX;
	_tempcoords._coords[1] = regionY;

	char** region = regions[_tempcoords._key];
	if (region == nullptr)
		return readChunk(x,y,out);

	char* chunk = region[chunkIndex];
	if (chunk == nullptr)
		return readChunk(x,y,out);
	for (unsigned int i = 0; i < CHUNK_VOL; i++)
		out[i] = chunk[i];
	return true;
}

bool WorldFiles::readChunk(int x, int y, char* out){
	assert(out != nullptr);

	int regionX = x >> REGION_SIZE_BIT;
	int regionY = y >> REGION_SIZE_BIT;

	int localX = x - (regionX << REGION_SIZE_BIT);
	int localY = y - (regionY << REGION_SIZE_BIT);
	int chunkIndex = localY * REGION_SIZE + localX;

	std::string filename = getRegionFile(regionX, regionY);

	std::ifstream input(filename, std::ios::binary);
	if (!input.is_open()){
		return false;
	}

	uint32_t offset;
	input.seekg(chunkIndex*4);
	input.read((char*)(&offset), 4);
	// Ordering bytes from big-endian to machine order (any, just reading)
	offset = bytes2Int((const unsigned char*)(&offset), 0);
	//assert (offset < 1000000);
	if (offset == 0){
		input.close();
		return false;
	}
	input.seekg(offset);
	input.read((char*)(&offset), 4);
	size_t compressedSize = bytes2Int((const unsigned char*)(&offset), 0);

	input.read(mainBufferIn, compressedSize);
	input.close();

	decompressRLE(mainBufferIn, compressedSize, out, CHUNK_VOL);

	return true;
}

void WorldFiles::write(){
	std::unordered_map<long, char**>::iterator it;
	for (it = regions.begin(); it != regions.end(); it++){
		if (it->second == nullptr)
			continue;

		int x;
		int y;
		longToCoords(x,y, it->first);

		unsigned int size = writeRegion(mainBufferOut, x,y, it->second);
		write_binary_file(getRegionFile(x,y), mainBufferOut, size);
	}
}

void WorldFiles::writePlayer(Player* player){
	char dst[1+3*4 + 1+2*4];

	glm::vec3 position = player->hitbox->position;

	size_t offset = 0;
	dst[offset++] = SECTION_POSITION;
	float2Bytes(position.x, dst, offset); offset += 4;
	float2Bytes(position.y, dst, offset); offset += 4;
	float2Bytes(position.z, dst, offset); offset += 4;

	dst[offset++] = SECTION_ROTATION;
	float2Bytes(player->camX, dst, offset); offset += 4;
	float2Bytes(player->camY, dst, offset); offset += 4;

	write_binary_file(getPlayerFile(), (const char*)dst, sizeof(dst));
}

bool WorldFiles::readPlayer(Player* player) {
	size_t length = 0;
	char* data = read_binary_file(getPlayerFile(), length);
	if (data == nullptr){
		std::cerr << "could not to read player.bin" << std::endl;
		return false;
	}
	glm::vec3 position = player->hitbox->position;
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
		}
	}
	player->hitbox->position = position;
	player->camera->position = position + vec3(0, 1, 0);
	return true;
}

unsigned int WorldFiles::writeRegion(char* out, int x, int y, char** region){
	unsigned int offset = REGION_VOL * 4;
	for (unsigned int i = 0; i < offset; i++)
		out[i] = 0;

	char* compressed = new char[CHUNK_VOL*2];
	for (int i = 0; i < REGION_VOL; i++){
		char* chunk = region[i];
		if (chunk == nullptr){
			chunk = new char[CHUNK_VOL];
			assert((((i % REGION_SIZE) + x * REGION_SIZE) >> REGION_SIZE_BIT) == x);
			assert((((i / REGION_SIZE) + y * REGION_SIZE) >> REGION_SIZE_BIT) == y);
			if (readChunk((i % REGION_SIZE) + x * REGION_SIZE, (i / REGION_SIZE) + y * REGION_SIZE, chunk)){
				region[i] = chunk;
				totalCompressed += CHUNK_VOL;
			} else {
				delete[] chunk;
				chunk = nullptr;
			}
		}

		if (chunk == nullptr){
			int2Bytes(0, out, i*4);
		} else {
			int2Bytes(offset, out, i*4);

			unsigned int compressedSize = compressRLE(chunk, CHUNK_VOL, compressed);

			int2Bytes(compressedSize, out, offset);
			offset += 4;

			for (unsigned int j = 0; j < compressedSize; j++)
				out[offset++] = compressed[j];
		}
	}
	delete[] compressed;
	return offset;
}

void longToCoords(int& x, int& y, long key) {
	_tempcoords._key = key;
	x = _tempcoords._coords[0];
	y = _tempcoords._coords[1];
}
