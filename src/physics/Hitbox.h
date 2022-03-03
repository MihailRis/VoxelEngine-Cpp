#ifndef PHYSICS_HITBOX_H_
#define PHYSICS_HITBOX_H_

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class Hitbox {
public:
	vec3 position;
	vec3 halfsize;
	vec3 velocity;
	float linear_damping;
	bool grounded = false;

	Hitbox(vec3 position, vec3 halfsize);
};

#endif /* PHYSICS_HITBOX_H_ */
