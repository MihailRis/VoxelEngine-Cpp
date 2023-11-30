#ifndef LOGIC_LEVEL_CONTROLLER_H_
#define LOGIC_LEVEL_CONTROLLER_H_

#include "../settings.h"

class Level;
class ChunksController;
class PlayerController;

class LevelController {
    EngineSettings& settings;

    Level* level;

    ChunksController* chunks;
    PlayerController* player;
public:
    LevelController(EngineSettings& settings, Level* level);
    ~LevelController();

    void update(float delta, 
                bool input, 
                bool pause, 
                bool interactions);
};

#endif // LOGIC_LEVEL_CONTROLLER_H_
