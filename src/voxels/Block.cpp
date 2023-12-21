#include "Block.h"

#include "../core_defs.h"

using glm::vec3;

CoordSystem::CoordSystem(glm::ivec3 axisX, glm::ivec3 axisY, glm::ivec3 axisZ, glm::ivec3 fix)
	: axisX(axisX), axisY(axisY), axisZ(axisZ), fix(fix)
{
	fix2 = glm::ivec3(0);
	if (isVectorHasNegatives(axisX)) fix2 -= axisX;
	if (isVectorHasNegatives(axisY)) fix2 -= axisY;
	if (isVectorHasNegatives(axisZ)) fix2 -= axisZ;
}

void CoordSystem::transform(AABB& aabb) {
	vec3 X(axisX);
	vec3 Y(axisY);
	vec3 Z(axisZ);
	aabb.a = X * aabb.a.x + Y * aabb.a.y + Z * aabb.a.z;
	aabb.b = X * aabb.b.x + Y * aabb.b.y + Z * aabb.b.z;
	aabb.a += fix2;
	aabb.b += fix2;
}

const BlockRotProfile BlockRotProfile::PIPE {"pipe", {//TODO consexpr or init-time fix calculations
		{ { 1, 0, 0 }, { 0, 0, 1 }, { 0, -1, 0 }, { 0, 0, -1 }}, // North
		{ { 0, 0, 1 }, {-1, 0, 0 }, { 0, -1, 0 }, { 1, 0, -1 }}, // East
		{ { -1, 0, 0 }, { 0, 0,-1 }, { 0, -1, 0 }, { 1, 0, 0 }}, // South
		{ { 0, 0, -1 }, { 1, 0, 0 }, { 0, -1, 0 }, { 0, 0, 0 }}, // West
		{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 }  }, // Up
		{ { 1, 0, 0 }, { 0,-1, 0 }, { 0, 0,-1 }, { 0, 1,-1 }  }, // Down
}};

const BlockRotProfile BlockRotProfile::PANE {"pane", {
		{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 }, { 0, 0, 0 }}, // North
		{ { 0, 0,-1 }, { 0, 1, 0 }, { 1, 0, 0 }, { 1, 0, 0 }}, // East
		{ {-1, 0, 0 }, { 0, 1, 0 }, { 0, 0,-1 }, { 1, 0,-1 }}, // South
		{ { 0, 0, 1 }, { 0, 1, 0 }, {-1, 0, 0 }, { 0, 0,-1 }}, // West
}};

Block::Block(std::string name) 
	: name(name), 
	  textureFaces {TEXTURE_NOTFOUND,TEXTURE_NOTFOUND,TEXTURE_NOTFOUND,
	  			    TEXTURE_NOTFOUND,TEXTURE_NOTFOUND,TEXTURE_NOTFOUND} {
	rotations = BlockRotProfile::PIPE;
}

Block::Block(std::string name, std::string texture) : name(name),
		textureFaces{texture,texture,texture,texture,texture,texture} {
	rotations = BlockRotProfile::PIPE;
}
