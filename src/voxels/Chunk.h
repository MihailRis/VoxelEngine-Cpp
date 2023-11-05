#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include <stdlib.h>
#include "../constants.h"

#define CHUNK_MODIFIED 0x1
#define CHUNK_READY 0x2
#define CHUNK_LOADED 0x4
#define CHUNK_LIGHTED 0x8
#define CHUNK_UNSAVED 0x10
#define CHUNK_DATA_LEN (CHUNK_VOL*2)

struct voxel;
class Lightmap;

struct RenderData {
	float* vertices;
	size_t size;
};

#define BIT_ON(f,i) do{f|= i;} while(0)
#define BIT_OFF(f,i) do{f&=~(i);} while(0)
#define BITSET(f,i,s) if (s) BIT_ON(f,i); else BIT_OFF(f,i);

class Chunk {
public:
	int x, z;
	voxel* voxels;
	Lightmap* lightmap;
	int flags = 0;
	int surrounding = 0;
	RenderData renderData;

	Chunk(int x, int z);
	~Chunk();

	bool isEmpty();

	Chunk* clone() const;

	// flags getters/setters below

	inline bool isUnsaved() const {return flags & CHUNK_UNSAVED;}

	inline bool isModified() const {return flags & CHUNK_MODIFIED;}

	inline bool isLighted() const {return flags & CHUNK_LIGHTED;}

	inline bool isLoaded() const {return flags & CHUNK_LOADED;}

	inline bool isReady() const {return flags & CHUNK_READY;}

	inline void setUnsaved(bool flag) {BITSET(flags, CHUNK_UNSAVED, flag);}

	inline void setModified(bool flag) {BITSET(flags, CHUNK_MODIFIED, flag);}

	inline void setLoaded(bool flag) {BITSET(flags, CHUNK_LOADED, flag);}

	inline void setLighted(bool flag) {BITSET(flags, CHUNK_LIGHTED, flag);}

	inline void setReady(bool flag) {BITSET(flags, CHUNK_READY, flag);}

	ubyte* encode() const;
	bool decode(ubyte* data);
};

#endif /* VOXELS_CHUNK_H_ */
