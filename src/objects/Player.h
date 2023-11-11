#ifndef SRC_OBJECTS_PLAYER_H_
#define SRC_OBJECTS_PLAYER_H_

#include <glm/glm.hpp>

#include "../voxels/voxel.h"

class Camera;
class Hitbox;
class PhysicsSolver;
class Chunks;

class Player {
public:
	float speed;
	Camera* camera;
	Hitbox* hitbox;
	bool flight = false;
	bool noclip = false;
	bool debug = false;
	int choosenBlock;
	float camX, camY;
	float cameraShaking = 0.0f;
	float cameraShakingTimer = 0.0f;
	glm::vec3 interpVel {0.0f, 0.0f, 0.0f};
	voxel selectedVoxel {0, 0};
	Player(glm::vec3 position, float speed, Camera* camera);
	~Player();

	void teleport(glm::vec3 position);
};

#endif /* SRC_OBJECTS_PLAYER_H_ */
