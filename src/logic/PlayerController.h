#ifndef PLAYER_CONTROL_H_
#define PLAYER_CONTROL_H_

#include <memory>
#include <glm/glm.hpp>

#include "../settings.h"
#include "../objects/Player.h"

class Camera;
class Level;
class BlocksController;

class CameraControl {
	Player* player;
	std::shared_ptr<Camera> camera, currentViewCamera;
	const CameraSettings& settings;
	glm::vec3 offset;
	float shake = 0.0f;
	float shakeTimer = 0.0f;
	glm::vec3 interpVel {0.0f};
public:
	CameraControl(Player* player, const CameraSettings& settings);
	void updateMouse(PlayerInput& input);
	void update(PlayerInput& input, float delta, Chunks* chunks);
	void refresh();
};

class PlayerController {
	Level* level;
	Player* player;
	PlayerInput input;
	CameraControl camControl;
    BlocksController* blocksController;

	void updateKeyboard();
	void updateCamera(float delta, bool movement);
	void resetKeyboard();
	void updateControls(float delta);
	void updateInteraction();
public:
	static glm::vec3 selectedBlockPosition;
	static glm::ivec3 selectedBlockNormal;
	static glm::vec3 selectedPointPosition;
	static int selectedBlockId;
	static int selectedBlockStates;

	PlayerController(Level* level, 
                     const EngineSettings& settings,
                     BlocksController* blocksController);
	void update(float delta, bool input, bool pause);
};

#endif /* PLAYER_CONTROL_H_ */
