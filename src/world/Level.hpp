#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include "typedefs.hpp"

inline constexpr float DEF_PLAYER_Y = 100.0f;
inline constexpr float DEF_PLAYER_SPEED = 4.0f;
inline constexpr int DEF_PLAYER_INVENTORY_SIZE = 40;

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
class Player;
struct EngineSettings;

/// @brief A level, contains chunks and objects
class Level {
    std::unique_ptr<World> world;
public:
    const Content* const content;
    std::unordered_map<int64_t, std::unique_ptr<Player>> players;
    std::unique_ptr<Chunks> chunks;
    std::unique_ptr<ChunksStorage> chunksStorage;
    std::unique_ptr<Inventories> inventories;

    std::unique_ptr<PhysicsSolver> physics;
    std::unique_ptr<Lighting> lighting;
    std::unique_ptr<LevelEvents> events;
    std::unique_ptr<Entities> entities;
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

    void addPlayer(std::unique_ptr<Player> player);

    Player* getPlayer(int64_t id) const;

    void onSave();

    std::shared_ptr<Camera> getCamera(const std::string& name);
};
