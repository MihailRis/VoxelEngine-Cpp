#include "Player.h"
#include "../content/ContentLUT.h"
#include "../physics/Hitbox.h"
#include "../physics/PhysicsSolver.h"
#include "../voxels/Chunks.h"
#include "../world/Level.h"
#include "../window/Events.h"
#include "../window/Camera.h"
#include "../items/Inventory.h"

#include <glm/glm.hpp>

const float CROUCH_SPEED_MUL = 0.35f;
const float RUN_SPEED_MUL = 1.5f;
const float PLAYER_GROUND_DAMPING = 10.0f;
const float PLAYER_AIR_DAMPING = 7.0f;
const float FLIGHT_SPEED_MUL = 4.0f;
const float CHEAT_SPEED_MUL = 5.0f;
const float JUMP_FORCE = 8.0f;

Player::Player(glm::vec3 position, float speed) :
		speed(speed),
		chosenSlot(0),
		inventory(new Inventory(40)),
	    camera(new Camera(position, glm::radians(90.0f))),
	    spCamera(new Camera(position, glm::radians(90.0f))),
	    tpCamera(new Camera(position, glm::radians(90.0f))),
        currentCamera(camera),
	    hitbox(new Hitbox(position, glm::vec3(0.3f,0.9f,0.3f)))
{
}

Player::~Player() {
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

	glm::vec3 dir(0,0,0);
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
	if (glm::length(dir) > 0.0f){
		dir = glm::normalize(dir);
		hitbox->velocity.x += dir.x * speed * delta * 9;
		hitbox->velocity.z += dir.z * speed * delta * 9;
	}

    float vel = std::max(glm::length(hitbox->velocity * 0.25f), 1.0f);
	int substeps = int(delta * vel * 1000);
	substeps = std::min(100, std::max(1, substeps));
	level->physics->step(level->chunks, hitbox.get(), 
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

	if (spawnpoint.y <= 0.1) {
		attemptToFindSpawnpoint(level);
	}
}

void Player::teleport(glm::vec3 position) {
	hitbox->position = position;
}

void Player::attemptToFindSpawnpoint(Level* level) {
	glm::vec3 ppos = hitbox->position;
	glm::vec3 newpos {ppos.x + (rand() % 200 - 100),
					  rand() % 80 + 100,
					  ppos.z + (rand() % 200 - 100)};
	while (newpos.y > 0 && !level->chunks->isObstacleBlock(newpos.x, newpos.y-2, newpos.z)) {
		newpos.y--;
	}

	voxel* headvox = level->chunks->get(newpos.x, newpos.y+1, newpos.z);
	if (level->chunks->isObstacleBlock(newpos.x, newpos.y, newpos.z) ||
		headvox == nullptr || headvox->id != 0)
		return;
	spawnpoint = newpos;
	teleport(spawnpoint);
}

void Player::setChosenSlot(int index) {
    chosenSlot = index;
}

int Player::getChosenSlot() const {
    return chosenSlot;
}

float Player::getSpeed() const {
	return speed;
}

std::shared_ptr<Inventory> Player::getInventory() const {
    return inventory;
}

void Player::setSpawnPoint(glm::vec3 spawnpoint) {
	this->spawnpoint = spawnpoint;
}

glm::vec3 Player::getSpawnPoint() const {
	return spawnpoint;
}

std::unique_ptr<dynamic::Map> Player::write() const {
	glm::vec3 position = hitbox->position;
	auto root = std::make_unique<dynamic::Map>();
	auto& posarr = root->putList("position");
	posarr.put(position.x);
	posarr.put(position.y);
	posarr.put(position.z);

	auto& rotarr = root->putList("rotation");
	rotarr.put(cam.x);
	rotarr.put(cam.y);

	auto& sparr = root->putList("spawnpoint");
	sparr.put(spawnpoint.x);
	sparr.put(spawnpoint.y);
	sparr.put(spawnpoint.z);

	root->put("flight", flight);
	root->put("noclip", noclip);
    root->put("chosen-slot", chosenSlot);
    root->put("inventory", inventory->write().release());
    return root;
}

void Player::convert(dynamic::Map* data, const ContentLUT* lut) {
    auto inventory = data->map("inventory");
    if (inventory) {
        Inventory::convert(inventory, lut);
    }
}
