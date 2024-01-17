#ifndef SRC_OBJECTS_PLAYER_H_
#define SRC_OBJECTS_PLAYER_H_

#include <memory>
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
    itemid_t chosenItem;
public:
	std::shared_ptr<Camera> camera, spCamera, tpCamera;
    std::shared_ptr<Camera> currentCamera;
	std::unique_ptr<Hitbox> hitbox;
	bool flight = false;
	bool noclip = false;
	bool debug = false;
	voxel selectedVoxel {0, 0};

	glm::vec2 cam = {};

	Player(glm::vec3 position, float speed);
	~Player() = default;

	void teleport(glm::vec3 position);
	void update(Level* level, PlayerInput& input, float delta);

    void setChosenItem(itemid_t id);

    itemid_t getChosenItem() const;
	float getSpeed() const;
};

#endif /* SRC_OBJECTS_PLAYER_H_ */
