#ifndef OBJECTS_ENTITY_DEF_HPP_
#define OBJECTS_ENTITY_DEF_HPP_

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "../typedefs.hpp"
#include "../maths/aabb.hpp"
#include "../physics/Hitbox.hpp"

namespace rigging {
    class RigConfig;
}

struct EntityDef {
    /// @brief Entity string id (with prefix included)
    std::string const name;

    std::vector<std::string> components;

    BodyType bodyType = BodyType::DYNAMIC;
    glm::vec3 hitbox {0.5f};
    std::vector<std::pair<size_t, AABB>> boxSensors {};
    std::vector<std::pair<size_t, float>> radialSensors {};
    std::string rigName = name;

    struct {
        bool enabled = true;
        struct {
            bool textures = false;
            bool pose = false;
        } rig;
    } save {};

    struct {
        entityid_t id;
        rigging::RigConfig* rig;
    } rt {};

    EntityDef(const std::string& name) : name(name) {}
    EntityDef(const EntityDef&) = delete;
};

#endif // OBJECTS_ENTITY_DEF_HPP_
