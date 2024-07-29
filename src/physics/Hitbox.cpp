#include "Hitbox.hpp"

#include <stdexcept>

std::optional<BodyType> BodyType_from(std::string_view str) {
    if (str == "kinematic") {
        return BodyType::KINEMATIC;
    } else if (str == "dynamic") {
        return BodyType::DYNAMIC;
    } else if (str == "static") {
        return BodyType::STATIC;
    }
    return std::nullopt;
}

std::string to_string(BodyType type) {
    switch (type) {
        case BodyType::KINEMATIC: return "kinematic";
        case BodyType::DYNAMIC: return "dynamic";
        case BodyType::STATIC: return "static";
        default: return "unknown";
    }
}

Hitbox::Hitbox(BodyType type, glm::vec3 position, glm::vec3 halfsize) 
  : type(type),
    position(position), 
    halfsize(halfsize), 
    velocity(0.0f,0.0f,0.0f), 
    linearDamping(0.1f) 
{}
