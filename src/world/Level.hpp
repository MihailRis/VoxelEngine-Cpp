#pragma once

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include "typedefs.hpp"

class Content;
class World;
class Chunks;
class Entities;
class Inventories;
class LevelEvents;
class Lighting;
class PhysicsSolver;
class ChunksStorage;
class Camera;
class Players;
struct EngineSettings;

/// @brief A level, contains chunks and objects
class Level {
    std::unique_ptr<World> world;
public:
    const Content* const content;
    
    std::unique_ptr<Chunks> chunks;
    std::unique_ptr<ChunksStorage> chunksStorage;
    std::unique_ptr<Inventories> inventories;

    std::unique_ptr<PhysicsSolver> physics;
    std::unique_ptr<Lighting> lighting;
    std::unique_ptr<LevelEvents> events;
    std::unique_ptr<Entities> entities;
    std::unique_ptr<Players> players;
    std::vector<std::shared_ptr<Camera>> cameras;  // move somewhere?

    const EngineSettings& settings;

    Level(
        std::unique_ptr<World> world,
        const Content* content,
        EngineSettings& settings
    );
    ~Level();

    void loadMatrix(int32_t x, int32_t z, uint32_t radius);

    World* getWorld();

    const World* getWorld() const;

    void onSave();

    std::shared_ptr<Camera> getCamera(const std::string& name);
};
