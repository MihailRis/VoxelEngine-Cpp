#ifndef OBJECTS_ENTITIES_HPP_
#define OBJECTS_ENTITIES_HPP_

#include "../typedefs.hpp"
#include "../physics/Hitbox.hpp"

#include <vector>
#include <optional>
#include <glm/glm.hpp>
#include <unordered_map>
#include <entt/entity/registry.hpp>

struct entity_funcs_set {
    bool init : 1;
    bool on_despawn : 1;
    bool on_grounded : 1;
    bool on_fall : 1;
    bool on_trigger_enter : 1;
    bool on_trigger_exit : 1;
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

    void refresh();
};

struct Rigidbody {
    bool enabled = true;
    Hitbox hitbox;
    std::vector<Trigger> triggers;
};

struct Scripting {
    entity_funcs_set funcsset;
    scriptenv env;
};

class Level;
class Assets;
class LineBatch;
class ModelBatch;
class Frustum;
class Rig;
class Entities;

class Entity {
    Entities& entities;
    entityid_t id;
    entt::registry& registry;
    const entt::entity entity;
public:
    Entity(Entities& entities, entityid_t id, entt::registry& registry, const entt::entity entity)
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

    Scripting& getScripting() const {
        return registry.get<Scripting>(entity);
    }

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
    entityid_t nextID = 1;
public:
    Entities(Level* level);

    void clean();
    void updatePhysics(float delta);
    void update();

    void renderDebug(LineBatch& batch, Frustum& frustum);
    void render(Assets* assets, ModelBatch& batch, Frustum& frustum);

    entityid_t spawn(EntityDef& def, glm::vec3 pos);

    std::optional<Entity> get(entityid_t id) {
        const auto& found = entities.find(id);
        if (found != entities.end() && registry.valid(found->second)) {
            return Entity(*this, id, registry, found->second);
        }
        return std::nullopt;
    }

    void despawn(entityid_t id);

    inline size_t size() const {
        return entities.size();
    }
};

#endif // OBJECTS_ENTITIES_HPP_
