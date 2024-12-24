#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <optional>

#include "interfaces/Serializable.hpp"
#include "settings.hpp"
#include "voxels/voxel.hpp"

class Chunks;
class Camera;
class Inventory;
class ContentReport;
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
};

struct CursorSelection {
    voxel vox {BLOCK_VOID, {}};
    glm::ivec3 position {};
    glm::ivec3 actualPosition {};
    glm::ivec3 normal {};
    glm::vec3 hitPosition;
    entityid_t entity = ENTITY_NONE;
};

class Player : public Serializable {
    Level* level;
    int64_t id;
    std::string name;
    float speed;
    int chosenSlot;
    glm::vec3 position;
    glm::vec3 spawnpoint {};
    std::shared_ptr<Inventory> inventory;
    bool flight = false;
    bool noclip = false;
    bool infiniteItems = true;
    bool instantDestruction = true;
    bool loadingChunks = true;
    entityid_t eid;
    entityid_t selectedEid = 0;
public:
    std::unique_ptr<Chunks> chunks;
    std::shared_ptr<Camera> fpCamera, spCamera, tpCamera;
    std::shared_ptr<Camera> currentCamera;;
    glm::vec3 cam {};
    CursorSelection selection {};

    Player(
        Level* level,
        int64_t id,
        const std::string& name,
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

    bool isInfiniteItems() const;
    void setInfiniteItems(bool flag);

    bool isInstantDestruction() const;
    void setInstantDestruction(bool flag);

    bool isLoadingChunks() const;
    void setLoadingChunks(bool flag);

    entityid_t getEntity() const;
    void setEntity(entityid_t eid);

    entityid_t getSelectedEntity() const;

    void setName(const std::string& name);
    const std::string& getName() const;

    const std::shared_ptr<Inventory>& getInventory() const;

    const glm::vec3& getPosition() const {
        return position;
    }

    Hitbox* getHitbox();

    void setSpawnPoint(glm::vec3 point);
    glm::vec3 getSpawnPoint() const;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;

    static void convert(dv::value& data, const ContentReport* report);

    inline u64id_t getId() const {
        return id;
    }
};
