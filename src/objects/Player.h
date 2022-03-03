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
	int choosenBlock;
	float camX, camY;
	Player(glm::vec3 position, float speed, Camera* camera);
	~Player();
};

#endif /* SRC_OBJECTS_PLAYER_H_ */
