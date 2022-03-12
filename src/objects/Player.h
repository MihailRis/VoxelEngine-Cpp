#ifndef SRC_OBJECTS_PLAYER_H_
#define SRC_OBJECTS_PLAYER_H_

#include <glm/glm.hpp>

class Camera;
class Hitbox;

class Player {
public:
	float speed;
	Camera* camera;
	Hitbox* hitbox;
	bool flight = false;
	int choosenBlock;
	float camX, camY;
	float cameraShaking = 0.0f;
	float cameraShakingTimer = 0.0f;
	glm::vec3 interpVel {0.0f, 0.0f, 0.0f};
	Player(glm::vec3 position, float speed, Camera* camera);
	~Player();
};

#endif /* SRC_OBJECTS_PLAYER_H_ */
