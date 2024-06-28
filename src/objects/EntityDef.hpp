#ifndef OBJECTS_ENTITY_DEF_HPP_
#define OBJECTS_ENTITY_DEF_HPP_

#include <string>
#include <glm/glm.hpp>

#include "../typedefs.hpp"

struct entity_funcs_set {
    bool init : 1;
    bool on_spawn : 1;
    bool on_despawn : 1;
};

struct EntityDef {
    /// @brief Entity string id (with prefix included)
    std::string const name;

    std::string scriptName = name.substr(name.find(':')+1);
    glm::vec3 hitbox {0.5f};
    
    struct {
        entityid_t id;
        entity_funcs_set funcsset;
    } rt;
    
    EntityDef(const std::string& name) : name(name) {}
    EntityDef(const EntityDef&) = delete;
};

#endif // OBJECTS_ENTITY_DEF_HPP_
