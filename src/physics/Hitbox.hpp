#pragma once

#include "maths/aabb.hpp"
#include "typedefs.hpp"

#include <set>
#include <string>
#include <optional>
#include <functional>
#include <glm/glm.hpp>

enum class SensorType {
    AABB,
    RADIUS,
};

union SensorParams {
    AABB aabb;
    glm::vec4 radial; // x,y,z calculated entity coords + w - radius

    constexpr SensorParams() : aabb() {
    }
};

using sensorcallback = std::function<void(entityid_t, size_t, entityid_t)>;

struct Sensor {
    bool enabled = true;
    SensorType type;
    size_t index;
    entityid_t entity;
    SensorParams params;
    SensorParams calculated;
    std::set<entityid_t> prevEntered;
    std::set<entityid_t> nextEntered;
    sensorcallback enterCallback;
    sensorcallback exitCallback;
};

enum class BodyType {
    STATIC, KINEMATIC, DYNAMIC
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

    AABB getAABB() const {
        return AABB(position-halfsize, position+halfsize);
    }
};
