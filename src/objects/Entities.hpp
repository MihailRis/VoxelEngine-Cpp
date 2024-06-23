#ifndef OBJECTS_ENTITIES_HPP_
#define OBJECTS_ENTITIES_HPP_

#include "../typedefs.hpp"

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

class Level;
class Assets;
class ModelBatch;

class Entity {
    entt::registry& registry;
    entt::entity entity;
public:
    Entity(entt::registry& registry, entt::entity entity)
    : registry(registry), entity(entity) {}

    bool isValid() const {
        return registry.valid(entity);
    }

    Transform& getTransform() const {
        return registry.get<Transform>(entity);
    }

    entityid_t getUID() const {
        return registry.get<EntityId>(entity).uid;
    }
};

class Entities {
    entt::registry registry;
    Level* level;
    std::unordered_map<entityid_t, entt::entity> entities;
public:
    Entities(Level* level);
    void updatePhysics(float delta);
    void render(Assets* assets, ModelBatch& batch);
};

#endif // OBJECTS_ENTITIES_HPP_
