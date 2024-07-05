#include "api_lua.hpp"

#include "../../LevelController.hpp"
#include "../../../world/Level.hpp"
#include "../../../objects/Player.hpp"
#include "../../../objects/Entities.hpp"
#include "../../../objects/rigging.hpp"
#include "../../../physics/Hitbox.hpp"
#include "../../../window/Camera.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../content/Content.hpp"
#include "../../../engine.hpp"

#include <optional>

namespace scripting {
    extern Hud* hud;
}
using namespace scripting;

static std::optional<Entity> get_entity(lua::State* L, int idx) {
    auto id = lua::tointeger(L, idx);
    auto level = controller->getLevel();
    return level->entities->get(id);
}

static int l_exists(lua::State* L) {
    return lua::pushboolean(L, get_entity(L, 1).has_value());
}

static int l_spawn(lua::State* L) {
    auto level = controller->getLevel();
    auto defname = lua::tostring(L, 1);
    auto& def = content->entities.require(defname);
    auto pos = lua::tovec3(L, 2);
    dynamic::Value args = dynamic::NONE;
    if (lua::gettop(L) > 2) {
        args = lua::tovalue(L, 3);
    }
    Transform transform {
        pos, glm::vec3(1.0f), glm::mat3(1.0f), {}, true
    };
    level->entities->spawn(scripting::engine->getAssets(), def, transform, args);
    return 1;
}

static int l_despawn(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->destroy();
    }
    return 0;
}

static int l_set_rig(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto assets = scripting::engine->getAssets();
        std::string rigName = lua::require_string(L, 2);
        auto rigConfig = assets->get<rigging::RigConfig>(rigName);
        if (rigConfig == nullptr) {
            throw std::runtime_error("rig not found '"+rigName+"'");
        }
        entity->setRig(rigConfig);
    }
    return 0;
}

static int l_get_pos(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getTransform().pos);
    }
    return 0;
}

static int l_set_pos(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getTransform().setPos(lua::tovec3(L, 2));
    }
    return 0;
}

static int l_get_vel(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getRigidbody().hitbox.velocity);
    }
    return 0;
}

static int l_set_vel(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.velocity = lua::tovec3(L, 2);
    }
    return 0;
}

static int l_get_rot(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushmat4(L, entity->getTransform().rot);
    }
    return 0;
}

static int l_set_rot(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getTransform().setRot(lua::tomat4(L, 2));
    }
    return 0;
}

static int l_is_enabled(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        lua::pushboolean(L, entity->getRigidbody().enabled);
    }
    return 0;
}

static int l_set_enabled(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().enabled = lua::toboolean(L, 2);
    }
    return 0;
}

static int l_get_size(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getRigidbody().hitbox.halfsize * 2.0f);
    }
    return 0;
}

static int l_set_size(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getRigidbody().hitbox.halfsize = lua::tovec3(L, 2) * 0.5f;
    }
    return 0;
}

static int index_range_check(const rigging::Rig& rig, lua::Integer index) {
    if (static_cast<size_t>(index) >= rig.pose.matrices.size()) {
        throw std::runtime_error("index out of range [0, " +
                                 std::to_string(rig.pose.matrices.size()) +
                                 "]");
    }
    return static_cast<int>(index);
}

static int l_modeltree_get_model(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& rig = entity->getModeltree();
        auto* rigConfig = rig.config;
        auto index = index_range_check(rig, lua::tointeger(L, 2));
        return lua::pushstring(L, rigConfig->getNodes()[index]->getModelName());
    }
    return 0;
}

static int l_modeltree_get_matrix(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& rig = entity->getModeltree();
        auto index = index_range_check(rig, lua::tointeger(L, 2));
        return lua::pushmat4(L, rig.pose.matrices[index]);
    }
    return 0;
}

static int l_modeltree_set_matrix(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& rig = entity->getModeltree();
        auto index = index_range_check(rig, lua::tointeger(L, 2));
        rig.pose.matrices[index] = lua::tomat4(L, 3);
    }
    return 0;
}

static int l_modeltree_set_texture(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        auto& rig = entity->getModeltree();
        rig.textures[lua::require_string(L, 2)] = lua::require_string(L, 3);
    }
    return 0;
}

const luaL_Reg entitylib [] = {
    {"exists", lua::wrap<l_exists>},
    {"spawn", lua::wrap<l_spawn>},
    {"despawn", lua::wrap<l_despawn>},
    {"set_rig", lua::wrap<l_set_rig>},
    {NULL, NULL}
};

const luaL_Reg modeltreelib [] = {
    {"get_model", lua::wrap<l_modeltree_get_model>},
    {"get_matrix", lua::wrap<l_modeltree_get_matrix>},
    {"set_matrix", lua::wrap<l_modeltree_set_matrix>},
    {"set_texture", lua::wrap<l_modeltree_set_texture>},
    {NULL, NULL}
};

const luaL_Reg transformlib [] = {
    {"get_pos", lua::wrap<l_get_pos>},
    {"set_pos", lua::wrap<l_set_pos>},
    {"get_size", lua::wrap<l_get_size>},
    {"set_size", lua::wrap<l_set_size>},
    {"get_rot", lua::wrap<l_get_rot>},
    {"set_rot", lua::wrap<l_set_rot>},
    {NULL, NULL}
};

const luaL_Reg rigidbodylib [] = {
    {"is_enabled", lua::wrap<l_is_enabled>},
    {"set_enabled", lua::wrap<l_set_enabled>},
    {"get_vel", lua::wrap<l_get_vel>},
    {"set_vel", lua::wrap<l_set_vel>},
    {"get_size", lua::wrap<l_get_size>},
    {"set_size", lua::wrap<l_set_size>},
    {NULL, NULL}
};
