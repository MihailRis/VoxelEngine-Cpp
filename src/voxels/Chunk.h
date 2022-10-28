#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#include <stdlib.h>

#define CHUNK_W 16
#define CHUNK_H 256
#define CHUNK_D 16
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

#define CHUNK_MODIFIED 0x1
#define CHUNK_READY 0x2
#define CHUNK_LOADED 0x4
#define CHUNK_LIGHTED 0x8
#define CHUNK_UNSAVED 0x10

class voxel;
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
	int references = 1;
	RenderData renderData;

	Chunk(int x, int z);
	~Chunk();

	bool isEmpty();

	Chunk* clone() const;
	void incref();
	void decref();

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
};

#endif /* VOXELS_CHUNK_H_ */
