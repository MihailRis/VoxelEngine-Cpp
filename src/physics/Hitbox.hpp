#ifndef PHYSICS_HITBOX_HPP_
#define PHYSICS_HITBOX_HPP_

#include "../maths/aabb.hpp"
#include "../typedefs.hpp"

#include <set>
#include <functional>
#include <glm/glm.hpp>

struct Trigger {
    bool enabled = true;
    entityid_t entity;
    AABB aabb;
    AABB calculated;
    std::set<entityid_t> prevEntered;
    std::set<entityid_t> nextEntered;
    std::function<void(entityid_t, size_t, entityid_t)> enterCallback;
    std::function<void(entityid_t, size_t, entityid_t)> exitCallback;
};

struct Hitbox {
    glm::vec3 position;
    glm::vec3 halfsize;
    glm::vec3 velocity;
    float linearDamping;
    bool grounded = false;

    Hitbox(glm::vec3 position, glm::vec3 halfsize);
};

#endif // PHYSICS_HITBOX_HPP_
