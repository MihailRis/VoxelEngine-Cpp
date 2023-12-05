#include "Player.h"
#include "../physics/Hitbox.h"
#include "../physics/PhysicsSolver.h"
#include "../voxels/Chunks.h"
#include "../world/Level.h"
#include "../window/Events.h"
#include "../window/Camera.h"

#include <glm/glm.hpp>

const float CROUCH_SPEED_MUL = 0.35f;
const float RUN_SPEED_MUL = 1.5f;
const float PLAYER_GROUND_DAMPING = 10.0f;
const float PLAYER_AIR_DAMPING = 7.0f;
const float FLIGHT_SPEED_MUL = 4.0f;
const float CHEAT_SPEED_MUL = 5.0f;
const float JUMP_FORCE = 8.0f;

Player::Player(glm::vec3 position, float speed, Camera* camera) :
		speed(speed),
		camera(camera),
		choosenBlock(1) {
	hitbox = new Hitbox(position, vec3(0.3f,0.9f,0.3f));
}

Player::~Player(){
	delete hitbox;
}

void Player::update(
		Level* level,
		PlayerInput& input, 
		float delta) {
	bool crouch = input.shift && hitbox->grounded && !input.sprint;
	float speed = this->speed;
	if (flight){
		speed *= FLIGHT_SPEED_MUL;
	}
	if (input.cheat){
		speed *= CHEAT_SPEED_MUL;
	}

	if (crouch) {
		speed *= CROUCH_SPEED_MUL;
	} else if (input.sprint) {
		speed *= RUN_SPEED_MUL;
	}

	vec3 dir(0,0,0);
	if (input.moveForward){
		dir.x += camera->dir.x;
		dir.z += camera->dir.z;
	}
	if (input.moveBack){
		dir.x -= camera->dir.x;
		dir.z -= camera->dir.z;
	}
	if (input.moveRight){
		dir.x += camera->right.x;
		dir.z += camera->right.z;
	}
	if (input.moveLeft){
		dir.x -= camera->right.x;
		dir.z -= camera->right.z;
	}
	if (length(dir) > 0.0f){
		dir = normalize(dir);
		hitbox->velocity.x += dir.x * speed * delta * 9;
		hitbox->velocity.z += dir.z * speed * delta * 9;
	}

	int substeps = (int)(delta * 1000);
	substeps = std::min(100, std::max(0, substeps));
	level->physics->step(level->chunks, hitbox, 
						 delta,  substeps, 
						 crouch, flight ? 0.0f : 1.0f, 
						 !noclip);
	if (flight && hitbox->grounded) {
		flight = false;
	}

	if (input.jump && hitbox->grounded){
		hitbox->velocity.y = JUMP_FORCE;
	}

	if ((input.flight && !noclip) ||
		(input.noclip && flight == noclip)){
		flight = !flight;
		if (flight){
			hitbox->grounded = false;
		}
	}
	if (input.noclip) {
		noclip = !noclip;
	}

	hitbox->linear_damping = PLAYER_GROUND_DAMPING;
	if (flight){
		hitbox->linear_damping = PLAYER_AIR_DAMPING;
		hitbox->velocity.y *= 1.0f - delta * 9;
		if (input.jump){
			hitbox->velocity.y += speed * delta * 9;
		}
		if (input.shift){
			hitbox->velocity.y -= speed * delta * 9;
		}
	}
	if (!hitbox->grounded) {
		hitbox->linear_damping = PLAYER_AIR_DAMPING;
	}

	input.noclip = false;
	input.flight = false;
}

void Player::teleport(glm::vec3 position) {
	hitbox->position = position;
}

float Player::getSpeed() const {
	return speed;
}
