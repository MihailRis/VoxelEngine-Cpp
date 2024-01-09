#ifndef SRC_OBJECTS_PLAYER_H_
#define SRC_OBJECTS_PLAYER_H_

#include <glm/glm.hpp>

#include "../voxels/voxel.h"
#include "../settings.h"

class Camera;
class Hitbox;
class PhysicsSolver;
class Chunks;
class Level;

struct PlayerInput {
	bool zoom;
	bool cameraMode;
	bool moveForward;
	bool moveBack;
	bool moveRight;
	bool moveLeft;
	bool sprint;
	bool shift;
	bool cheat;
	bool jump;
	bool noclip;
	bool flight;
};

class Player {
	float speed;
public:
	Camera* camera, *SPCamera, *TPCamera, *currentViewCamera;
	Hitbox* hitbox;
	bool flight = false;
	bool noclip = false;
	bool debug = false;
	int chosenBlock;
	voxel selectedVoxel {0};

	glm::vec2 cam = {};

	Player(glm::vec3 position, float speed);
	~Player();

	void teleport(glm::vec3 position);

	float getSpeed() const;
	void update(Level* level, PlayerInput& input, float delta);
};

#endif /* SRC_OBJECTS_PLAYER_H_ */
