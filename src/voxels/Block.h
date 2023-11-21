#ifndef VOXELS_BLOCK_H_
#define VOXELS_BLOCK_H_

#include <string>

#define FACE_MX 0
#define FACE_PX 1
#define FACE_MY 2
#define FACE_PY 3
#define FACE_MZ 4
#define FACE_PZ 5

enum class BlockModel {
	none, block, xsprite
};

class Block {
public:
	std::string const name;
	unsigned int id;
						               //  0 1   2 3   4 5
	std::string textureFaces[6]; // -x,x, -y,y, -z,z
	unsigned char emission[3];
	unsigned char drawGroup = 0;
	BlockModel model = BlockModel::block;
	bool lightPassing = false;
	bool skyLightPassing = false;
	bool obstacle = true;
	bool selectable = true;
	bool breakable = true;
	bool rotatable = false;
	float hitboxScale = 1;

	float uvdata[4*6];

	Block(std::string name, std::string texture);
};

#endif /* VOXELS_BLOCK_H_ */
