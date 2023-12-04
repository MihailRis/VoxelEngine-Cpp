#include "Block.h"

using glm::vec3;

void CoordSystem::transform(AABB& aabb) {
	vec3 X(axisX);
	vec3 Y(axisY);
	vec3 Z(axisZ);
	aabb.a = X * aabb.a.x + Y * aabb.a.y + Z * aabb.a.z;
	aabb.b = X * aabb.b.x + Y * aabb.b.y + Z * aabb.b.z;
	aabb.a += fix2;
	aabb.b += fix2;
}

const BlockRotProfile BlockRotProfile::PIPE {{
		// Vertical
		{{1, 0, 0}, {0, 1, 0}, {0, 0, 1},	{0, 0, 0}, {0, 0, 0}},
		// X-Aligned
		{{0, -1, 0}, {1, 0, 0}, {0, 0, 1},	{0, 1, 0}, {0, 1, 0}},
		// Z-Aligned
		{{1, 0, 0}, {0, 0, 1}, {0, -1, 0},	{0, 0, -1}, {0, 1, 0}},
}};

Block::Block(std::string name) 
	: name(name), 
	  textureFaces {"notfound","notfound","notfound",
	  			    "notfound","notfound","notfound",} {
	rotations = BlockRotProfile::PIPE;
}

Block::Block(std::string name, std::string texture) : name(name),
		textureFaces{texture,texture,texture,texture,texture,texture} {
}
