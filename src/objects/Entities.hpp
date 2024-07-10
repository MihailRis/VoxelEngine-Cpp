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
    bool init : 1;
    bool on_despawn : 1;
    bool on_grounded : 1;
    bool on_fall : 1;
    bool on_sensor_enter : 1;
    bool on_sensor_exit : 1;
    bool on_save : 1;
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

    void loadEntities(dynamic::Map_sptr map);
    void loadEntity(const dynamic::Map_sptr& map);
    void loadEntity(const dynamic::Map_sptr& map, Entity entity);
    void onSave(const Entity& entity);
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
