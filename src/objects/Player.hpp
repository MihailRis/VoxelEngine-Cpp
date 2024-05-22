#ifndef SRC_OBJECTS_PLAYER_HPP_
#define SRC_OBJECTS_PLAYER_HPP_

#include "../settings.hpp"
#include "../data/dynamic.hpp"
#include "../voxels/voxel.hpp"
#include "../interfaces/Serializable.hpp"
#include "../interfaces/Object.hpp"

#include <memory>
#include <glm/glm.hpp>

class Camera;
class Hitbox;
class Inventory;
class ContentLUT;
class Level;
struct EngineSettings;

struct PlayerInput {
    bool zoom : 1;
    bool cameraMode : 1;
    bool moveForward : 1;
    bool moveBack : 1;
    bool moveRight : 1;
    bool moveLeft : 1;
    bool sprint : 1;
    bool shift : 1;
    bool cheat : 1;
    bool jump : 1;
    bool noclip : 1;
    bool flight : 1;
};

class Player : public Object, public Serializable {
    float speed;
    int chosenSlot;
    glm::vec3 spawnpoint {};
    std::shared_ptr<Inventory> inventory;
public:
    std::shared_ptr<Camera> camera, spCamera, tpCamera;
    std::shared_ptr<Camera> currentCamera;
    std::unique_ptr<Hitbox> hitbox;
    bool flight = false;
    bool noclip = false;
    bool debug = false;
    voxel selectedVoxel {0, 0};
    glm::vec2 cam {};

    Player(glm::vec3 position, float speed, std::shared_ptr<Inventory> inv);
    ~Player();

    void teleport(glm::vec3 position);
    void updateInput(Level* level, PlayerInput& input, float delta);

    void attemptToFindSpawnpoint(Level* level);

    void setChosenSlot(int index);

    int getChosenSlot() const;
    float getSpeed() const;
    
    std::shared_ptr<Inventory> getInventory() const;

    void setSpawnPoint(glm::vec3 point);
    glm::vec3 getSpawnPoint() const;

    std::unique_ptr<dynamic::Map> serialize() const override;
    void deserialize(dynamic::Map *src) override;

    static void convert(dynamic::Map* data, const ContentLUT* lut);

    inline int getId() const {
        return objectUID;
    }
};

#endif // SRC_OBJECTS_PLAYER_HPP_
