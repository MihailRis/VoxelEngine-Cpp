#ifndef PLAYER_CONTROL_H_
#define PLAYER_CONTROL_H_

#include <glm/glm.hpp>

class PhysicsSolver;
class Chunks;
class Player;
class Level;

class PlayerController {
	Level* level;
public:
	glm::vec3 selectedBlockPosition;
	int selectedBlockId = -1;
	PlayerController(Level* level);
	void update_controls(float delta, bool movement);
	void update_interaction();
};

#endif /* PLAYER_CONTROL_H_ */
