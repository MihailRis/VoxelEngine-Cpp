#ifndef SRC_OBJECTS_PLAYER_H_
#define SRC_OBJECTS_PLAYER_H_

#include <memory>
#include <glm/glm.hpp>

#include "../data/dynamic.h"
#include "../voxels/voxel.h"
#include "../settings.h"
#include "../interfaces/Serializable.h"
#include "../interfaces/Object.h"

class Camera;
class Hitbox;
class Inventory;
class ContentLUT;
class PhysicsSolver;
class ChunksMatrix;
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
    std::unique_ptr<ChunksMatrix> chunksMatrix;
    const EngineSettings& settings;
    uint32_t radius = 0;
    bool flight = false;
    bool noclip = false;
    bool debug = false;
    voxel selectedVoxel {0, 0};

    glm::vec2 cam = {};

    Player(Level* level, glm::vec3 position, float speed, std::shared_ptr<Inventory> inv, const EngineSettings& settings);
    ~Player();

    void teleport(glm::vec3 position);
    void updateInput(Level* level, PlayerInput& input, float delta);
    void loadChunks();

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

#endif /* SRC_OBJECTS_PLAYER_H_ */
