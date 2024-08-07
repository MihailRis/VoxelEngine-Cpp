#ifndef OBJECTS_ENTITY_DEF_HPP_
#define OBJECTS_ENTITY_DEF_HPP_

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include <typedefs.hpp>
#include <maths/aabb.hpp>
#include <physics/Hitbox.hpp>

namespace rigging {
    class SkeletonConfig;
}

struct EntityDef {
    /// @brief Entity string id (with prefix included)
    std::string const name;

    /// @brief Component IDs
    std::vector<std::string> components;

    /// @brief Physic body type
    BodyType bodyType = BodyType::DYNAMIC;

    /// @brief Hitbox size
    glm::vec3 hitbox {0.25f};
    
    /// @brief 'aabb' sensors
    std::vector<std::pair<size_t, AABB>> boxSensors {};
    /// @brief 'radius' sensors
    std::vector<std::pair<size_t, float>> radialSensors {};
    
    /// @brief Skeleton ID
    std::string skeletonName = name;

    /// @brief Does entity prevent blocks setup
    bool blocking = true;

    /// @brief save-** flags
    struct {
        bool enabled = true;
        struct {
            bool textures = false;
            bool pose = false;
        } skeleton;
        struct {
            bool velocity = true;
            bool settings = false;
        } body;
    } save {};

    struct {
        entityid_t id;
        rigging::SkeletonConfig* skeleton;
    } rt {};

    EntityDef(const std::string& name) : name(name) {}
    EntityDef(const EntityDef&) = delete;
};

#endif // OBJECTS_ENTITY_DEF_HPP_
