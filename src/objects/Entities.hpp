#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <vector>

#include "data/dv.hpp"
#include "physics/Hitbox.hpp"
#include "typedefs.hpp"
#include "util/Clock.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <entt/entity/registry.hpp>
#include <glm/gtx/norm.hpp>
#include <unordered_map>

struct EntityFuncsSet {
    bool init;
    bool on_despawn;
    bool on_grounded;
    bool on_fall;
    bool on_sensor_enter;
    bool on_sensor_exit;
    bool on_save;
    bool on_aim_on;
    bool on_aim_off;
    bool on_attacked;
    bool on_used;
};

struct EntityDef;

struct EntityId {
    entityid_t uid;
    const EntityDef& def;
    bool destroyFlag = false;
    int64_t player = -1;
};

struct Transform {
    static inline constexpr float EPSILON = 0.0000001f;
    glm::vec3 pos;
    glm::vec3 size;
    glm::mat3 rot;
    glm::mat4 combined;
    bool dirty = true;

    glm::vec3 displayPos;
    glm::vec3 displaySize;

    void refresh();

    inline void setRot(glm::mat3 m) {
        rot = m;
        dirty = true;
    }

    inline void setSize(glm::vec3 v) {
        if (glm::distance2(displaySize, v) >= EPSILON) {
            dirty = true;
        }
        size = v;
    }

    inline void setPos(glm::vec3 v) {
        if (glm::distance2(displayPos, v) >= EPSILON) {
            dirty = true;
        }
        pos = v;
    }
};

struct Rigidbody {
    bool enabled = true;
    Hitbox hitbox;
    std::vector<Sensor> sensors;
};

struct UserComponent {
    std::string name;
    EntityFuncsSet funcsset;
    scriptenv env;

    UserComponent(
        const std::string& name, EntityFuncsSet funcsset, scriptenv env
    )
        : name(name), funcsset(funcsset), env(env) {
    }
};

struct ScriptComponents {
    std::vector<std::unique_ptr<UserComponent>> components;

    ScriptComponents() = default;

    ScriptComponents(ScriptComponents&& other)
        : components(std::move(other.components)) {
    }
};

class Level;
class Assets;
class LineBatch;
class ModelBatch;
class Frustum;
class Entities;
class DrawContext;

namespace rigging {
    struct Skeleton;
    class SkeletonConfig;
}

class Entity {
    Entities& entities;
    entityid_t id;
    entt::registry& registry;
    const entt::entity entity;
public:
    Entity(
        Entities& entities,
        entityid_t id,
        entt::registry& registry,
        const entt::entity entity
    )
        : entities(entities), id(id), registry(registry), entity(entity) {
    }

    EntityId& getID() const {
        return registry.get<EntityId>(entity);
    }

    bool isValid() const {
        return registry.valid(entity);
    }

    const EntityDef& getDef() const {
        return registry.get<EntityId>(entity).def;
    }

    Transform& getTransform() const {
        return registry.get<Transform>(entity);
    }

    Rigidbody& getRigidbody() const {
        return registry.get<Rigidbody>(entity);
    }

    ScriptComponents& getScripting() const {
        return registry.get<ScriptComponents>(entity);
    }

    rigging::Skeleton& getSkeleton() const;

    void setRig(const rigging::SkeletonConfig* rigConfig);

    entityid_t getUID() const {
        return registry.get<EntityId>(entity).uid;
    }

    entt::entity getHandler() const {
        return entity;
    }

    int64_t getPlayer() const {
        return registry.get<EntityId>(entity).player;
    }

    void setPlayer(int64_t id) {
        registry.get<EntityId>(entity).player = id;
    }

    void setInterpolatedPosition(const glm::vec3& position);

    glm::vec3 getInterpolatedPosition() const;

    void destroy();
};

class Entities {
    entt::registry registry;
    Level& level;
    std::unordered_map<entityid_t, entt::entity> entities;
    std::unordered_map<entt::entity, entityid_t> uids;
    entityid_t nextID = 1;
    util::Clock sensorsTickClock;
    util::Clock updateTickClock;

    void updateSensors(
        Rigidbody& body, const Transform& tsf, std::vector<Sensor*>& sensors
    );
    void preparePhysics(float delta);
public:
    struct RaycastResult {
        entityid_t entity;
        glm::ivec3 normal;
        float distance;
    };

    Entities(Level& level);

    void clean();
    void updatePhysics(float delta);
    void update(float delta);

    void renderDebug(
        LineBatch& batch, const Frustum* frustum, const DrawContext& ctx
    );
    void render(
        const Assets& assets,
        ModelBatch& batch,
        const Frustum* frustum,
        float delta,
        bool pause
    );

    entityid_t spawn(
        const EntityDef& def,
        glm::vec3 position,
        dv::value args = nullptr,
        dv::value saved = nullptr,
        entityid_t uid = 0
    );

    std::optional<Entity> get(entityid_t id) {
        const auto& found = entities.find(id);
        if (found != entities.end() && registry.valid(found->second)) {
            return Entity(*this, id, registry, found->second);
        }
        return std::nullopt;
    }

    /// @brief Entities raycast. No blocks check included, use combined with
    /// Chunks.rayCast
    /// @param start Ray start
    /// @param dir Ray direction normalized vector
    /// @param maxDistance Max ray length
    /// @param ignore Ignored entity ID
    /// @return An optional structure containing entity, normal and distance
    std::optional<RaycastResult> rayCast(
        glm::vec3 start,
        glm::vec3 dir,
        float maxDistance,
        entityid_t ignore = -1
    );

    void loadEntities(dv::value map);
    void loadEntity(const dv::value& map);
    void loadEntity(const dv::value& map, Entity entity);
    void onSave(const Entity& entity);
    bool hasBlockingInside(AABB aabb);
    std::vector<Entity> getAllInside(AABB aabb);
    std::vector<Entity> getAllInRadius(glm::vec3 center, float radius);
    void despawn(entityid_t id);
    void despawn(std::vector<Entity> entities);
    dv::value serialize(const Entity& entity);
    dv::value serialize(const std::vector<Entity>& entities);

    void setNextID(entityid_t id) {
        nextID = id;
    }

    inline size_t size() const {
        return entities.size();
    }

    inline entityid_t peekNextID() const {
        return nextID;
    }
};
