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
	void step(Chunks* chunks,
			Hitbox* hitbox,
			float delta,
			unsigned substeps,
			bool shifting,
			float gravityScale,
			bool collisions);
	void colisionCalc(Chunks* chunks, Hitbox* hitbox, vec3* vel, vec3* pos, vec3 half);
	bool isBlockInside(int x, int y, int z, Hitbox* hitbox);
};

#endif /* PHYSICS_PHYSICSSOLVER_H_ */
