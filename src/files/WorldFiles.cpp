#include "WorldFiles.h"

#include "files.h"
#include "../window/Camera.h"
#include "../objects/Player.h"
#include "../physics/Hitbox.h"
#include "../voxels/voxel.h"
#include "../voxels/Chunk.h"
#include "../typedefs.h"
#include "../voxmaths.h"

#include <cassert>
#include <iostream>
#include <cstdint>
#include <fstream>
#include <iostream>

#define SECTION_POSITION 1
#define SECTION_ROTATION 2
#define SECTION_FLAGS 3
#define PLAYER_FLAG_FLIGHT 0x1
#define PLAYER_FLAG_NOCLIP 0x2
#define CHUNK_DATA_LEN (CHUNK_VOL*sizeof(voxel))

using glm::ivec2;
using glm::vec3;

int64_t WorldFiles::totalCompressed = 0;

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
	mainBufferIn = new ubyte[CHUNK_DATA_LEN];
	mainBufferOut = new ubyte[mainBufferCapacity];
}

WorldFiles::~WorldFiles(){
	delete[] mainBufferIn;
	delete[] mainBufferOut;
	for (auto it = regions.begin(); it != regions.end(); it++){
	    WorldRegion region = it->second;
	    if (region.chunksData == nullptr)
	    	continue;
	    for (unsigned int i = 0; i < REGION_VOL; i++){
	    	delete[] region.chunksData[i];
	    }
	    delete[] region.chunksData;
	}
	regions.clear();
}

void WorldFiles::put(const ubyte* chunkData, int x, int y){
	assert(chunkData != nullptr);

	int regionX = floordiv(x, REGION_SIZE);
	int regionY = floordiv(y, REGION_SIZE);

	int localX = x - (regionX * REGION_SIZE);
	int localY = y - (regionY * REGION_SIZE);

	ivec2 key(regionX, regionY);

	auto found = regions.find(key);
	if (found == regions.end()) {
		ubyte** chunksData = new ubyte*[REGION_VOL];
		for (uint i = 0; i < REGION_VOL; i++) {
			chunksData[i] = nullptr;
		}
		regions[key] = { chunksData, true };
	}

	WorldRegion& region = regions[key];
	region.unsaved = true;
	ubyte* targetChunk = region.chunksData[localY * REGION_SIZE + localX];
	if (targetChunk == nullptr){
		targetChunk = new ubyte[CHUNK_DATA_LEN];
		region.chunksData[localY * REGION_SIZE + localX] = targetChunk;
		totalCompressed += CHUNK_DATA_LEN;
	}
	for (uint i = 0; i < CHUNK_DATA_LEN; i++)
		targetChunk[i] = chunkData[i];

}

std::string WorldFiles::getRegionFile(int x, int y) {
	return directory + std::to_string(x) + "_" + std::to_string(y) + ".bin";
}

std::string WorldFiles::getPlayerFile() {
	return directory + "/player.bin";
}

bool WorldFiles::getChunk(int x, int y, ubyte* out){
	assert(out != nullptr);

	int regionX = floordiv(x, REGION_SIZE);
	int regionY = floordiv(y, REGION_SIZE);

	int localX = x - (regionX * REGION_SIZE);
	int localY = y - (regionY * REGION_SIZE);

	int chunkIndex = localY * REGION_SIZE + localX;
	assert(chunkIndex >= 0 && chunkIndex < REGION_VOL);

	ivec2 key(regionX, regionY);

	auto found = regions.find(key);
	if (found == regions.end()) {
		return readChunk(x, y, out);
	}

	WorldRegion& region = found->second;
	ubyte* chunk = region.chunksData[chunkIndex];
	if (chunk == nullptr)
		return readChunk(x,y,out);
	for (uint i = 0; i < CHUNK_DATA_LEN; i++)
		out[i] = chunk[i];
	return true;
}

bool WorldFiles::readChunk(int x, int y, ubyte* out){
	assert(out != nullptr);

	int regionX = floordiv(x, REGION_SIZE);
	int regionY = floordiv(y, REGION_SIZE);

	int localX = x - (regionX * REGION_SIZE);
	int localY = y - (regionY * REGION_SIZE);

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
	size_t compressedSize = bytes2Int((const ubyte*)(&offset), 0);
	input.read((char*)mainBufferIn, compressedSize);
	input.close();

	decompressRLE((ubyte*)mainBufferIn, compressedSize, (ubyte*)out, CHUNK_DATA_LEN);

	return true;
}

void WorldFiles::write(){
	for (auto it = regions.begin(); it != regions.end(); it++){
		if (it->second.chunksData == nullptr || !it->second.unsaved)
			continue;

		ivec2 key = it->first;

		unsigned int size = writeRegion(mainBufferOut, key.x, key.y, it->second.chunksData);
		write_binary_file(getRegionFile(key.x, key.y), (const char*)mainBufferOut, size);
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

	write_binary_file(getPlayerFile(), (const char*)dst, sizeof(dst));
}

bool WorldFiles::readPlayer(Player* player) {
	size_t length = 0;
	ubyte* data = (ubyte*)read_binary_file(getPlayerFile(), length);
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
				unsigned char flags = data[offset++];
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

uint WorldFiles::writeRegion(ubyte* out, int x, int y, ubyte** region){
	uint offset = REGION_VOL * 4;
	for (uint i = 0; i < offset; i++)
		out[i] = 0;

	ubyte* compressed = new ubyte[CHUNK_DATA_LEN];
	for (int i = 0; i < REGION_VOL; i++){
		ubyte* chunk = region[i];
		if (chunk == nullptr){
			chunk = new ubyte[CHUNK_DATA_LEN];
			if (readChunk((i % REGION_SIZE) + x * REGION_SIZE, (i / REGION_SIZE) + y * REGION_SIZE, chunk)){
				region[i] = chunk;
				totalCompressed += CHUNK_DATA_LEN;
			} else {
				delete[] chunk;
				chunk = nullptr;
			}
		}

		if (chunk == nullptr){
			int2Bytes(0, out, i*4);
		} else {
			int2Bytes(offset, out, i*4);

			uint compressedSize = compressRLE(chunk, CHUNK_DATA_LEN, compressed);

			int2Bytes(compressedSize, out, offset);
			offset += 4;

			for (uint j = 0; j < compressedSize; j++)
				out[offset++] = compressed[j];
		}
	}
	delete[] compressed;
	return offset;
}
