#ifndef LOGIC_LEVEL_CONTROLLER_H_
#define LOGIC_LEVEL_CONTROLLER_H_

#include <memory>
#include "../settings.h"

#include "PlayerController.h"
#include "BlocksController.h"
#include "ChunksController.h"

class Level;
class Player;

/* LevelController manages other controllers */
class LevelController {
    EngineSettings& settings;
    std::unique_ptr<Level> level;
    // Sub-controllers
    std::unique_ptr<BlocksController> blocks;
    std::unique_ptr<ChunksController> chunks;
    std::unique_ptr<PlayerController> player;
public:
    LevelController(EngineSettings& settings, Level* level);

    /* 
    @param delta time elapsed since the last update
    @param input is user input allowed to be handled
    @param pause is world and player simulation paused
    */
    void update(float delta, 
                bool input, 
                bool pause);

    void onWorldSave();
    void onWorldQuit();

    Level* getLevel();
    Player* getPlayer();
};

#endif // LOGIC_LEVEL_CONTROLLER_H_
