#ifndef PHYSICS_PHYSICSSOLVER_H_
#define PHYSICS_PHYSICSSOLVER_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Hitbox;
class Chunks;

class PhysicsSolver {
	vec3 gravity;
public:
	PhysicsSolver(vec3 gravity);
	void step(Chunks* chunks, Hitbox* hitbox, float delta, unsigned substeps, bool shifting, float gravityScale);
	bool isBlockInside(int x, int y, int z, Hitbox* hitbox);
};

#endif /* PHYSICS_PHYSICSSOLVER_H_ */
