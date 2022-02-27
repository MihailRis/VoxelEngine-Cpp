#ifndef VOXELS_CHUNK_H_
#define VOXELS_CHUNK_H_

#define CHUNK_W 16
#define CHUNK_H 256
#define CHUNK_D 16
#define CHUNK_VOL (CHUNK_W * CHUNK_H * CHUNK_D)

class voxel;
class Lightmap;

class Chunk {
public:
	int x,y,z;
	voxel* voxels;
	Lightmap* lightmap;
	bool modified = true;
	bool ready = false;
	bool accepted = false;
	bool generated = false;
	Chunk(int x, int y, int z);
	~Chunk();

	bool isEmpty();

	Chunk* clone() const;
};

#endif /* VOXELS_CHUNK_H_ */
