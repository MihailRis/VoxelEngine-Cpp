#ifndef SRC_OBJECTS_PLAYER_HPP_
#define SRC_OBJECTS_PLAYER_HPP_

#include "../data/dynamic.hpp"
#include "../voxels/voxel.hpp"
#include "../settings.hpp"
#include "../interfaces/Serializable.hpp"
#include "../interfaces/Object.hpp"

#include <memory>
#include <glm/glm.hpp>

class Camera;
class Hitbox;
class Inventory;
class ContentLUT;
class PhysicsSolver;
class Chunks;
class Level;

struct PlayerInput {
    bool zoom;
    bool cameraMode;
    bool moveForward;
    bool moveBack;
    bool moveRight;
    bool moveLeft;
    bool sprint;
    bool shift;
    bool cheat;
    bool jump;
    bool noclip;
    bool flight;
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

    glm::vec2 cam = {};

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

#endif /* SRC_OBJECTS_PLAYER_HPP_ */
