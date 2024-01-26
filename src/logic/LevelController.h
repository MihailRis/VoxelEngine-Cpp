#ifndef LOGIC_LEVEL_CONTROLLER_H_
#define LOGIC_LEVEL_CONTROLLER_H_

#include <memory>
#include "../settings.h"

class Level;
class BlocksController;
class ChunksController;
class PlayerController;

/* LevelController manages other controllers */
class LevelController {
    EngineSettings& settings;
    Level* level;
    // Sub-controllers
    std::unique_ptr<BlocksController> blocks;
    std::unique_ptr<ChunksController> chunks;
    std::unique_ptr<PlayerController> player;
public:
    LevelController(EngineSettings& settings, Level* level);
    ~LevelController();

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
};

#endif // LOGIC_LEVEL_CONTROLLER_H_
