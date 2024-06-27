#include "api_lua.hpp"

#include "../../LevelController.hpp"
#include "../../../world/Level.hpp"
#include "../../../objects/Player.hpp"
#include "../../../objects/Entities.hpp"
#include "../../../physics/Hitbox.hpp"
#include "../../../window/Camera.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../content/Content.hpp"

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

static int l_spawn(lua::State* L) {
    auto level = controller->getLevel();
    auto defname = lua::tostring(L, 1);
    auto& def = content->entities.require(defname);
    auto pos = lua::tovec3(L, 2);
    auto id = level->entities->spawn(def, pos);
    return lua::pushinteger(L, id);
}

static int l_get_pos(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getTransform().pos);
    }
    return 0;
}

static int l_set_pos(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getTransform().pos = lua::tovec3(L, 2);
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
        entity->getTransform().rot = lua::tomat4(L, 2);
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

const luaL_Reg entitylib [] = {
    {"spawn", lua::wrap<l_spawn>},
    {NULL, NULL}
};

 const luaL_Reg transformlib [] = {
    {"get_pos", lua::wrap<l_get_pos>},
    {"set_pos", lua::wrap<l_set_pos>},
    {"get_rot", lua::wrap<l_get_rot>},
    {"set_rot", lua::wrap<l_set_rot>},
    {NULL, NULL}
 };

const luaL_Reg rigidbodylib [] = {
    {"is_enabled", lua::wrap<l_is_enabled>},
    {"set_enabled", lua::wrap<l_set_enabled>},
    {"get_vel", lua::wrap<l_get_vel>},
    {"set_vel", lua::wrap<l_set_vel>},
    {NULL, NULL}
};
