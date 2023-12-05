#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include <stdlib.h>
#include "../constants.h"

struct ChunkFlag{
	static const int MODIFIED = 0x1;
	static const int READY = 0x2;
	static const int LOADED = 0x4;
	static const int LIGHTED = 0x8;
	static const int UNSAVED = 0x10;
};
#define CHUNK_DATA_LEN (CHUNK_VOL*2)

struct voxel;
class Lightmap;

struct RenderData {
	float* vertices;
	size_t size;
};

class Chunk {
public:
	int x, z;
	int bottom, top;
	voxel* voxels;
	Lightmap* lightmap;
	int flags = 0;
	int surrounding = 0;
	RenderData renderData;

	Chunk(int x, int z);
	~Chunk();

	bool isEmpty();

	void updateHeights();

	Chunk* clone() const;

	// flags getters/setters below
	
	void SETFLAGS(int mask, bool value){
		if (value)
			flags |= mask; 
		else
			flags &= ~(mask);
	}

	inline bool isUnsaved() const {return flags & ChunkFlag::UNSAVED;}

	inline bool isModified() const {return flags & ChunkFlag::MODIFIED;}

	inline bool isLighted() const {return flags & ChunkFlag::LIGHTED;}

	inline bool isLoaded() const {return flags & ChunkFlag::LOADED;}

	inline bool isReady() const {return flags & ChunkFlag::READY;}

	inline void setUnsaved(bool newState) {SETFLAGS(ChunkFlag::UNSAVED, newState);}

	inline void setModified(bool newState) {SETFLAGS(ChunkFlag::MODIFIED, newState);}

	inline void setLoaded(bool newState) {SETFLAGS(ChunkFlag::LOADED, newState);}

	inline void setLighted(bool newState) {SETFLAGS(ChunkFlag::LIGHTED, newState);}

	inline void setReady(bool newState) {SETFLAGS(ChunkFlag::READY, newState);}

	ubyte* encode() const;
	bool decode(ubyte* data);
};

#endif /* VOXELS_CHUNK_H_ */
