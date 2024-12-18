#pragma once

#include <string>
#include <vector>

class Engine;
class World;
class LevelController;

class EngineController {
    Engine& engine;
public:
    EngineController(Engine& engine);

    /// @brief Load world, convert if required and set to LevelScreen.
    /// @param name world name
    /// @param confirmConvert automatically confirm convert if requested
    void openWorld(const std::string& name, bool confirmConvert);

    /// @brief Show world removal confirmation dialog
    /// @param name world name
    void deleteWorld(const std::string& name);

    void reconfigPacks(
        LevelController* controller,
        const std::vector<std::string>& packsToAdd,
        const std::vector<std::string>& packsToRemove
    );

    void createWorld(
        const std::string& name,
        const std::string& seedstr,
        const std::string& generatorID
    );

    void reopenWorld(World* world);
};
