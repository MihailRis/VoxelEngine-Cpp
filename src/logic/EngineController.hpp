#ifndef LOGIC_ENGINE_CONTROLLER_HPP_
#define LOGIC_ENGINE_CONTROLLER_HPP_

#include <string>
#include <vector>

class Engine;
class World;
class LevelController;

class EngineController {
    Engine* engine;
public:
    EngineController(Engine* engine);

    /// @brief Load world, convert if required and set to LevelScreen.
    /// @param name world name
    /// @param confirmConvert automatically confirm convert if requested
    void openWorld(std::string name, bool confirmConvert);

    /// @brief Show world removal confirmation dialog
    /// @param name world name
    void deleteWorld(std::string name);

    void reconfigPacks(
        LevelController* controller,
        std::vector<std::string> packsToAdd,
        std::vector<std::string> packsToRemove
    );

    void createWorld(
        const std::string& name, 
        const std::string& seedstr,
        const std::string& generatorID
    );

    void reopenWorld(World* world);
};

#endif // LOGIC_ENGINE_CONTROLLER_HPP_
