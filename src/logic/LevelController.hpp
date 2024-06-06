#ifndef LOGIC_LEVEL_CONTROLLER_HPP_
#define LOGIC_LEVEL_CONTROLLER_HPP_

#include <memory>

#include "PlayerController.hpp"
#include "BlocksController.hpp"
#include "ChunksController.hpp"

class Level;
class Player;
struct EngineSettings;

/// @brief LevelController manages other controllers
class LevelController {
    EngineSettings& settings;
    std::unique_ptr<Level> level;
    // Sub-controllers
    std::unique_ptr<BlocksController> blocks;
    std::unique_ptr<ChunksController> chunks;
    std::unique_ptr<PlayerController> player;
public:
    LevelController(EngineSettings& settings, std::unique_ptr<Level> level);

    /// @param delta time elapsed since the last update
    /// @param input is user input allowed to be handled
    /// @param pause is world and player simulation paused
    void update(
        float delta,
        bool input, 
        bool pause
    );

    void saveWorld();
    
    void onWorldQuit();

    Level* getLevel();
    Player* getPlayer();

    BlocksController* getBlocksController();
    PlayerController* getPlayerController();
};

#endif // LOGIC_LEVEL_CONTROLLER_HPP_
