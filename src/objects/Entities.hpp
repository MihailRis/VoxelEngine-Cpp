#ifndef OBJECTS_ENTITIES_HPP_
#define OBJECTS_ENTITIES_HPP_

#include "../typedefs.hpp"
#include "../physics/Hitbox.hpp"

#include <optional>
#include <glm/glm.hpp>
#include <unordered_map>
#include <entt/entity/registry.hpp>

struct EntityId {
    entityid_t uid;
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
};

class Level;
class Assets;
class LineBatch;
class ModelBatch;
class Frustum;
class Rig;
struct EntityDef;

class Entity {
    entt::registry& registry;
    const entt::entity entity;
public:
    Entity(entt::registry& registry, const entt::entity entity)
    : registry(registry), entity(entity) {}

    bool isValid() const {
        return registry.valid(entity);
    }

    Transform& getTransform() const {
        return registry.get<Transform>(entity);
    }

    Rigidbody& getRigidbody() const {
        return registry.get<Rigidbody>(entity);
    }

    entityid_t getUID() const {
        return registry.get<EntityId>(entity).uid;
    }
};

class Entities {
    entt::registry registry;
    Level* level;
    std::unordered_map<entityid_t, entt::entity> entities;
    entityid_t nextID = 1;
public:
    Entities(Level* level);
    void updatePhysics(float delta);

    void renderDebug(LineBatch& batch);
    void render(Assets* assets, ModelBatch& batch, Frustum& frustum);

    entityid_t spawn(EntityDef& def, glm::vec3 pos);

    std::optional<Entity> get(entityid_t id) {
        const auto& found = entities.find(id);
        if (found != entities.end()) {
            return Entity(registry, found->second);
        }
        return std::nullopt;
    }
};

#endif // OBJECTS_ENTITIES_HPP_
