#ifndef WORLD_LEVEL_HPP_
#define WORLD_LEVEL_HPP_

#include "../objects/Objects.hpp"

#include <memory>
#include <vector>

inline constexpr float DEF_PLAYER_Y = 100.0f;
inline constexpr float DEF_PLAYER_SPEED = 4.0f;
inline constexpr int DEF_PLAYER_INVENTORY_SIZE = 40;

class Content;
class World;
class Player;
class Chunks;
class Inventory;
class Inventories;
class LevelEvents;
class Lighting;
class PhysicsSolver;
class ChunksStorage;
struct EngineSettings;

/// @brief A level, contains chunks and objects
class Level {
    std::unique_ptr<World> world;
public:
    Objects objects;
    const Content* const content;
    std::unique_ptr<Chunks> chunks;
    std::unique_ptr<ChunksStorage> chunksStorage;
    std::unique_ptr<Inventories> inventories;

    std::unique_ptr<PhysicsSolver> physics;
    std::unique_ptr<Lighting> lighting;
    std::unique_ptr<LevelEvents> events;

    const EngineSettings& settings;

    Level(
        std::unique_ptr<World> world, 
        const Content* content, 
        EngineSettings& settings
    );
    ~Level();

    void loadMatrix(int32_t x, int32_t z, uint32_t radius);
    
    World* getWorld();
};

#endif /* WORLD_LEVEL_HPP_ */
