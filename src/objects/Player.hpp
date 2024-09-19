#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <optional>

#include "interfaces/Object.hpp"
#include "interfaces/Serializable.hpp"
#include "settings.hpp"
#include "voxels/voxel.hpp"

class Camera;
class Inventory;
class ContentLUT;
class Level;
struct Hitbox;
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

struct CursorSelection {
    voxel vox {BLOCK_VOID, {}};
    glm::ivec3 position {};
    glm::ivec3 actualPosition {};
    glm::ivec3 normal {};
    glm::vec3 hitPosition;
    entityid_t entity = ENTITY_NONE;
};

class Player : public Object, public Serializable {
    Level* level;
    float speed;
    int chosenSlot;
    glm::vec3 position;
    glm::vec3 spawnpoint {};
    std::shared_ptr<Inventory> inventory;
    bool flight = false;
    bool noclip = false;
    entityid_t eid;
    entityid_t selectedEid;
public:
    std::shared_ptr<Camera> camera, spCamera, tpCamera;
    std::shared_ptr<Camera> currentCamera;
    bool debug = false;
    glm::vec3 cam {};
    CursorSelection selection {};

    Player(
        Level* level,
        glm::vec3 position,
        float speed,
        std::shared_ptr<Inventory> inv,
        entityid_t eid
    );
    ~Player();

    void teleport(glm::vec3 position);
    void updateEntity();
    void updateInput(PlayerInput& input, float delta);
    void updateSelectedEntity();
    void postUpdate();

    void attemptToFindSpawnpoint();

    void setChosenSlot(int index);

    int getChosenSlot() const;
    float getSpeed() const;

    bool isFlight() const;
    void setFlight(bool flag);

    bool isNoclip() const;
    void setNoclip(bool flag);

    entityid_t getEntity() const;
    void setEntity(entityid_t eid);

    entityid_t getSelectedEntity() const;

    std::shared_ptr<Inventory> getInventory() const;

    glm::vec3 getPosition() const {
        return position;
    }

    Hitbox* getHitbox();

    void setSpawnPoint(glm::vec3 point);
    glm::vec3 getSpawnPoint() const;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;

    static void convert(dv::value& data, const ContentLUT* lut);

    inline int getId() const {
        return objectUID;
    }
};
