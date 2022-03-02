#include "Player.h"
#include "../physics/Hitbox.h"

#include <glm/glm.hpp>

Player::Player(glm::vec3 position, float speed, Camera* camera) :
		speed(speed),
		camera(camera){
	hitbox = new Hitbox(position, vec3(0.2f,0.9f,0.2f));
}
