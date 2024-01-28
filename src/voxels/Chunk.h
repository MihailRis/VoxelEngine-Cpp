#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include <stdlib.h>
#include "../constants.h"

struct ChunkFlag {
	static const int MODIFIED = 0x1;
	static const int READY = 0x2;
	static const int LOADED = 0x4;
	static const int LIGHTED = 0x8;
	static const int UNSAVED = 0x10;
	static const int LOADED_LIGHTS = 0x20;
};
#define CHUNK_DATA_LEN (CHUNK_VOL*4)

struct voxel;
class Lightmap;
class ContentLUT;

class Chunk {
public:
	int x, z;
	int bottom, top;
	voxel* voxels;
	Lightmap* lightmap;
	int flags = 0;

	Chunk(int x, int z);
	~Chunk();

	bool isEmpty();

	void updateHeights();

	Chunk* clone() const;

	// flags getters/setters below
	inline void setFlags(int mask, bool value){
		if (value)
			flags |= mask; 
		else
			flags &= ~(mask);
	}

	inline bool isUnsaved() const {return flags & ChunkFlag::UNSAVED;}

	inline bool isModified() const {return flags & ChunkFlag::MODIFIED;}

	inline bool isLighted() const {return flags & ChunkFlag::LIGHTED;}

	inline bool isLoaded() const {return flags & ChunkFlag::LOADED;}

	inline bool isLoadedLights() const {return flags & ChunkFlag::LOADED_LIGHTS;}

	inline bool isReady() const {return flags & ChunkFlag::READY;}

	inline void setUnsaved(bool newState) {setFlags(ChunkFlag::UNSAVED, newState);}

	inline void setModified(bool newState) {setFlags(ChunkFlag::MODIFIED, newState);}

	inline void setLoaded(bool newState) {setFlags(ChunkFlag::LOADED, newState);}

	inline void setLoadedLights(bool newState) {setFlags(ChunkFlag::LOADED_LIGHTS, newState);}

	inline void setLighted(bool newState) {setFlags(ChunkFlag::LIGHTED, newState);}

	inline void setReady(bool newState) {setFlags(ChunkFlag::READY, newState);}

	ubyte* encode() const;
	bool decode(ubyte* data);

    static void convert(ubyte* data, const ContentLUT* lut);
};

#endif /* VOXELS_CHUNK_H_ */
