#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <string>
#include <glm/glm.hpp>

#include "../maths/aabb.h"
#include "../typedefs.h"

constexpr uint FACE_MX = 0;
constexpr uint FACE_PX = 1;
constexpr uint FACE_MY = 2;
constexpr uint FACE_PY = 3;
constexpr uint FACE_MZ = 4;
constexpr uint FACE_PZ = 5;

constexpr uint BLOCK_AABB_GRID = 16;

struct block_funcs_set {
	bool init: 1;
	bool update: 1;
    bool onplaced: 1;
    bool onbroken: 1;
    bool oninteract: 1;
    bool randupdate: 1;
};

struct CoordSystem {
	glm::ivec3 axisX;
	glm::ivec3 axisY;
	glm::ivec3 axisZ;

	// Grid 3d position fix offset (for negative vectors)
	glm::ivec3 fix;

	CoordSystem() = default;
	CoordSystem(glm::ivec3 axisX, glm::ivec3 axisY, glm::ivec3 axisZ);

	void transform(AABB& aabb) const;

	static bool isVectorHasNegatives(glm::ivec3 vec) {
		if (vec.x < 0 || vec.y < 0 || vec.z < 0) {
			return true;
		}
		else return false;
	}
};

struct BlockRotProfile {
	static const int MAX_COUNT = 16;
	std::string name;
	CoordSystem variants[MAX_COUNT];

	/* Wood logs, pillars, pipes */
	static const BlockRotProfile PIPE;
	/* Doors, signs and other panes */
	static const BlockRotProfile PANE;
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
    bool grounded = false;
    bool hidden = false;
	AABB hitbox;
	BlockRotProfile rotations;

	struct {
		blockid_t id;
		bool solid = true;
		bool emissive = false;
		AABB hitboxes[BlockRotProfile::MAX_COUNT];
		block_funcs_set funcsset {};
	} rt;

	Block(std::string name);
	Block(std::string name, std::string texture);
};

#endif /* VOXELS_BLOCK_H_ */
