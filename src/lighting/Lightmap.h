#ifndef LIGHTING_LIGHTMAP_H_
#define LIGHTING_LIGHTMAP_H_

#include "../voxels/Chunk.h"

class Lightmap {
public:
	unsigned short* map;
	Lightmap();
	~Lightmap();

	inline unsigned char get(int x, int y, int z, int channel){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] >> (channel << 2)) & 0xF;
	}

	inline unsigned char getR(int x, int y, int z){
		return map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] & 0xF;
	}

	inline unsigned char getG(int x, int y, int z){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] >> 4) & 0xF;
	}

	inline unsigned char getB(int x, int y, int z){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] >> 8) & 0xF;
	}

	inline unsigned char getS(int x, int y, int z){
		return (map[y*CHUNK_D*CHUNK_W+z*CHUNK_W+x] >> 12) & 0xF;
	}

	inline void setR(int x, int y, int z, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & 0xFFF0) | value;
	}

	inline void setG(int x, int y, int z, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & 0xFF0F) | (value << 4);
	}

	inline void setB(int x, int y, int z, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & 0xF0FF) | (value << 8);
	}

	inline void setS(int x, int y, int z, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & 0x0FFF) | (value << 12);
	}

	inline void set(int x, int y, int z, int channel, int value){
		const int index = y*CHUNK_D*CHUNK_W+z*CHUNK_W+x;
		map[index] = (map[index] & (0xFFFF & (~(0xF << (channel*4))))) | (value << (channel << 2));
	}
};

#endif /* LIGHTING_LIGHTMAP_H_ */
