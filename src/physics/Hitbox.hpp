#ifndef PHYSICS_HITBOX_HPP_
#define PHYSICS_HITBOX_HPP_

#include "../maths/aabb.hpp"
#include "../typedefs.hpp"

#include <set>
#include <string>
#include <optional>
#include <functional>
#include <glm/glm.hpp>

enum class TriggerType {
    AABB,
    RADIUS,
};

union TriggerParams {
    AABB aabb;
    glm::vec4 radial; // x,y,z calculated entity coords + w - radius

    constexpr TriggerParams() : aabb() {
    }
};

using triggercallback = std::function<void(entityid_t, size_t, entityid_t)>;

struct Trigger {
    bool enabled = true;
    TriggerType type;
    size_t index;
    entityid_t entity;
    TriggerParams params;
    TriggerParams calculated;
    std::set<entityid_t> prevEntered;
    std::set<entityid_t> nextEntered;
    triggercallback enterCallback;
    triggercallback exitCallback;
};

enum class BodyType {
    KINEMATIC, DYNAMIC
};

std::optional<BodyType> BodyType_from(std::string_view str);
std::string to_string(BodyType type);

struct Hitbox {
    BodyType type;
    glm::vec3 position;
    glm::vec3 halfsize;
    glm::vec3 velocity;
    float linearDamping;
    bool verticalDamping = false;
    bool grounded = false;
    float gravityScale = 1.0f;
    bool crouching = false;

    Hitbox(BodyType type, glm::vec3 position, glm::vec3 halfsize);
};

#endif // PHYSICS_HITBOX_HPP_
