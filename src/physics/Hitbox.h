#ifndef PHYSICS_HITBOX_H_
#define PHYSICS_HITBOX_H_

#include <glm/glm.hpp>

class Hitbox {
public:
	glm::vec3 position;
	glm::vec3 halfsize;
	glm::vec3 velocity;
	float linear_damping;
	bool grounded = false;

	Hitbox(glm::vec3 position, glm::vec3 halfsize);
};

#endif /* PHYSICS_HITBOX_H_ */
