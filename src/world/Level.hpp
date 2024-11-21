#pragma once

#include <memory>
#include <vector>

#include "interfaces/Object.hpp"

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
struct EngineSettings;

/// @brief A level, contains chunks and objects
class Level {
    std::unique_ptr<World> world;
public:
    const Content* const content;
    std::vector<std::shared_ptr<Object>> objects;
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

    /// Spawns object of class T and returns pointer to it.
    /// @param T class that derives the Object class
    /// @param args pass arguments needed for T class constructor
    template <class T, typename... Args>
    std::shared_ptr<T> spawnObject(Args&&... args) {
        static_assert(
            std::is_base_of<Object, T>::value,
            "T must be a derived of Object class"
        );
        std::shared_ptr<T> tObj = std::make_shared<T>(args...);

        std::shared_ptr<Object> obj =
            std::dynamic_pointer_cast<Object, T>(tObj);
        obj->objectUID = objects.size();
        objects.push_back(obj);
        obj->spawned();
        return tObj;
    }

    template <class T>
    std::shared_ptr<T> getObject(uint64_t id) const {
        static_assert(
            std::is_base_of<Object, T>::value,
            "T must be a derived of Object class"
        );
        if (id >= objects.size()) return nullptr;
        std::shared_ptr<T> object = std::dynamic_pointer_cast<T>(objects[id]);
        return object;
    }

    void onSave();

    std::shared_ptr<Camera> getCamera(const std::string& name);
};
