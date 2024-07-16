#ifndef OBJECTS_ENTITIES_HPP_
#define OBJECTS_ENTITIES_HPP_

#include "../typedefs.hpp"
#include "../physics/Hitbox.hpp"
#include "../data/dynamic.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include <unordered_map>
#include <entt/entity/registry.hpp>

struct entity_funcs_set {
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
};

struct Transform {
    glm::vec3 pos;
    glm::vec3 size;
    glm::mat3 rot;
    glm::mat4 combined;
    bool dirty = true;

    void refresh();
    
    inline void setRot(glm::mat3 m) {
        rot = m;
        dirty = true;
    }

    inline void setSize(glm::vec3 v) {
        if (glm::distance2(size, v) >= 0.0000001f) {
            dirty = true;
        }
        size = v;
    }

    inline void setPos(glm::vec3 v) {
        if (glm::distance2(pos, v) >= 0.00001f) {
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
    entity_funcs_set funcsset;
    scriptenv env;

    UserComponent(const std::string& name, entity_funcs_set funcsset, scriptenv env)
      : name(name), funcsset(funcsset), env(env) {}
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
    : entities(entities), id(id), registry(registry), entity(entity) {}

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

    void destroy();
};

class Entities {
    entt::registry registry;
    Level* level;
    std::unordered_map<entityid_t, entt::entity> entities;
    std::unordered_map<entt::entity, entityid_t> uids;
    entityid_t nextID = 1;

    void preparePhysics();
public:
    struct RaycastResult {
        entityid_t entity;
        glm::ivec3 normal;
        float distance;        
    };

    Entities(Level* level);

    void clean();
    void updatePhysics(float delta);
    void update();

    void renderDebug(LineBatch& batch, const Frustum& frustum);
    void render(Assets* assets, ModelBatch& batch, const Frustum& frustum, bool pause);

    entityid_t spawn(
        EntityDef& def,
        glm::vec3 position,
        dynamic::Value args=dynamic::NONE,
        dynamic::Map_sptr saved=nullptr,
        entityid_t uid=0);

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
        glm::vec3 start, glm::vec3 dir, float maxDistance, entityid_t ignore=-1);

    void loadEntities(dynamic::Map_sptr map);
    void loadEntity(const dynamic::Map_sptr& map);
    void loadEntity(const dynamic::Map_sptr& map, Entity entity);
    void onSave(const Entity& entity);
    bool hasBlockingInside(AABB aabb);
    std::vector<Entity> getAllInside(AABB aabb);
    void despawn(entityid_t id);
    dynamic::Value serialize(const Entity& entity);

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

#endif // OBJECTS_ENTITIES_HPP_
