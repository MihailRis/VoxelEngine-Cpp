#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <string>
#include <glm/glm.hpp>

#include "../maths/aabb.h"
#include "../typedefs.h"

const uint FACE_MX = 0;
const uint FACE_PX = 1;
const uint FACE_MY = 2;
const uint FACE_PY = 3;
const uint FACE_MZ = 4;
const uint FACE_PZ = 5;

const uint BLOCK_AABB_GRID = 16;

struct CoordSystem {
	glm::ivec3 axisX;
	glm::ivec3 axisY;
	glm::ivec3 axisZ;
	// Grid 3d position fix offset (for negative vectors)
	glm::ivec3 fix;
	glm::ivec3 fix2;

	void transform(AABB& aabb);
};

struct BlockRotProfile {
	static const int MAX_COUNT = 16;
	CoordSystem variants[MAX_COUNT];

	/* Wood logs, pillars, pipes
	   3 orientations supported
	 */
	static const BlockRotProfile PIPE;
};

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
	unsigned char emission[4] {0, 0, 0, 0};
	unsigned char drawGroup = 0;
	BlockModel model = BlockModel::block;
	bool lightPassing = false;
	bool skyLightPassing = false;
	bool obstacle = true;
	bool selectable = true;
	bool replaceable = false;
	bool breakable = true;
	bool rotatable = false;
	AABB hitbox;
	BlockRotProfile rotations;

	struct {
		blockid_t id;
		bool solid = true;
		bool emissive = false;
		AABB hitboxes[BlockRotProfile::MAX_COUNT];
	} rt;

	Block(std::string name);
	Block(std::string name, std::string texture);
};

#endif /* VOXELS_BLOCK_H_ */
