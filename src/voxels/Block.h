#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <string>

#include "../maths/aabb.h"
#include "../typedefs.h"

#define FACE_MX 0
#define FACE_PX 1
#define FACE_MY 2
#define FACE_PY 3
#define FACE_MZ 4
#define FACE_PZ 5

#define BLOCK_AABB_GRID 16

enum class BlockModel {
	none, // invisible 
	block, // default shape
	xsprite, // X-shape (grass)
	aabb // box shaped as block hitbox
};

class Block {
public:
	std::string const name;
						               //  0 1   2 3   4 5
	std::string textureFaces[6]; // -x,x, -y,y, -z,z
	unsigned char emission[4];
	unsigned char drawGroup = 0;
	BlockModel model = BlockModel::block;
	bool lightPassing = false;
	bool skyLightPassing = false;
	bool obstacle = true;
	bool selectable = true;
	bool breakable = true;
	bool rotatable = false;
	AABB hitbox;

	struct {
		blockid_t id;
		bool solid = true;
		bool emissive = false;
		bool hitboxGrid[BLOCK_AABB_GRID][BLOCK_AABB_GRID][BLOCK_AABB_GRID];
	} rt;

	Block(std::string name, std::string texture);
};

#endif /* VOXELS_BLOCK_H_ */
