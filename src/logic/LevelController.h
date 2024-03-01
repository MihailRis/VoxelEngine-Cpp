#ifndef LOGIC_LEVEL_CONTROLLER_H_
#define LOGIC_LEVEL_CONTROLLER_H_

#include <memory>
#include "../interfaces/Object.h"
#include "../settings.h"

class Level;
class BlocksController;
class ChunksController;
class PlayerController;

/* LevelController manages other controllers */
class LevelController : public Object {
    EngineSettings& settings;
    Level* level;
    // Sub-controllers
    std::shared_ptr<BlocksController> blocks;
    std::shared_ptr<ChunksController> chunks;
    std::shared_ptr<PlayerController> player;
public:
    LevelController(EngineSettings& settings, Level* level);
    ~LevelController();

    /* 
    @param delta time elapsed since the last update
    @param input is user input allowed to be handled
    @param pause is world and player simulation paused
    */
    void updateLevel(float delta, 
                bool input, 
                bool pause);

    void onWorldSave();
    void onWorldQuit();
};

#endif // LOGIC_LEVEL_CONTROLLER_H_
