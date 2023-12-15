#include "Block.h"

#include "../core_defs.h"

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

const BlockRotProfile BlockRotProfile::PIPE {"pipe", {//TODO consexpr or init-time fix and fix2 calculations
		{ { 1, 0, 0 }, { 0, 0, 1 }, { 0, -1, 0 }, { 0, 0, -1 }, { 0, 1, 0 } }, // North
		{ { 0, 0, 1 }, {-1, 0, 0 }, { 0, -1, 0 }, { 1, 0, -1 }, { 1, 1, 0 } }, // East
		{ { -1, 0, 0 }, { 0, 0,-1 }, { 0, -1, 0 }, { 1, 0, 0 }, { 1, 1, 1 } }, // South
		{ { 0, 0, -1 }, { 1, 0, 0 }, { 0, -1, 0 }, { 0, 0, 0 }, { 0, 1, 1 } }, // West
		{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 }, { 0, 0, 0 } }, // Up
		{ { 1, 0, 0 }, { 0,-1, 0 }, { 0, 0,-1 }, { 0, 1,-1 }, { 0, 1, 1 } }, // Down
}};

const BlockRotProfile BlockRotProfile::PANE {"pane", {
		{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 }, { 0, 0, 0 } }, // North
		{ { 0, 0,-1 }, { 0, 1, 0 }, { 1, 0, 0 }, { 1, 0, 0 }, { 0, 0, 1 } }, // East
		{ {-1, 0, 0 }, { 0, 1, 0 }, { 0, 0,-1 }, { 1, 0,-1 }, { 1, 0, 1 } }, // South
		{ { 0, 0, 1 }, { 0, 1, 0 }, {-1, 0, 0 }, { 0, 0,-1 }, { 1, 0, 0 } }, // West
}};

Block::Block(std::string name) 
	: name(name), 
	  textureFaces {TEXTURE_NOTFOUND,TEXTURE_NOTFOUND,TEXTURE_NOTFOUND,
	  			    TEXTURE_NOTFOUND,TEXTURE_NOTFOUND,TEXTURE_NOTFOUND,} {
	rotations = BlockRotProfile::PIPE;
}

Block::Block(std::string name, std::string texture) : name(name),
		textureFaces{texture,texture,texture,texture,texture,texture} {
	rotations = BlockRotProfile::PIPE;
}
