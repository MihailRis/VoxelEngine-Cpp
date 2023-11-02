#ifndef PHYSICS_PHYSICSSOLVER_H_
#define PHYSICS_PHYSICSSOLVER_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Hitbox;
class Chunks;

class PhysicsSolver {
	glm::vec3 gravity;
public:
	PhysicsSolver(glm::vec3 gravity);
	void step(Chunks* chunks,
			Hitbox* hitbox,
			float delta,
			unsigned substeps,
			bool shifting,
			float gravityScale,
			bool collisions);
	void colisionCalc(Chunks* chunks, Hitbox* hitbox, glm::vec3* vel, glm::vec3* pos, glm::vec3 half);
	bool isBlockInside(int x, int y, int z, Hitbox* hitbox);
};

#endif /* PHYSICS_PHYSICSSOLVER_H_ */
