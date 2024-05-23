#include "lua_commons.hpp"
#include "api_lua.hpp"
#include "../scripting.hpp"
#include "../../../world/Level.hpp"
#include "../../../objects/Player.hpp"
#include "../../../physics/Hitbox.hpp"
#include "../../../window/Camera.hpp"
#include "../../../items/Inventory.hpp"

#include <glm/glm.hpp>

inline std::shared_ptr<Player> get_player(lua_State* L, int idx) {
    return scripting::level->getObject<Player>(lua_tointeger(L, idx));
}

static int l_player_get_pos(lua_State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    glm::vec3 pos = player->hitbox->position;
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

static int l_player_set_pos(lua_State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    auto x = lua_tonumber(L, 2);
    auto y = lua_tonumber(L, 3);
    auto z = lua_tonumber(L, 4);
    player->hitbox->position = glm::vec3(x, y, z);
    return 0;
}

static int l_player_get_vel(lua_State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    glm::vec3 vel = player->hitbox->velocity;
    lua_pushnumber(L, vel.x);
    lua_pushnumber(L, vel.y);
    lua_pushnumber(L, vel.z);
    return 3;
}

static int l_player_set_vel(lua_State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    auto x = lua_tonumber(L, 2);
    auto y = lua_tonumber(L, 3);
    auto z = lua_tonumber(L, 4);
    player->hitbox->velocity = glm::vec3(x, y, z);
    return 0;
}

static int l_player_get_rot(lua_State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    const glm::vec3& rot = player->cam;
    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    lua_pushnumber(L, rot.z);
    return 3;
}

static int l_player_set_rot(lua_State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    glm::vec3& cam = player->cam;

    lua_Number x = lua_tonumber(L, 2);
    lua_Number y = lua_tonumber(L, 3);
    lua_Number z = cam.z;
    if (lua_isnumber(L, 4)) {
        z = lua_tonumber(L, 4);
    }
    cam.x = x;
    cam.y = y;
    cam.z = z;
    return 0;
}

static int l_player_get_inv(lua_State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    lua_pushinteger(L, player->getInventory()->getId());
    lua_pushinteger(L, player->getChosenSlot());
    return 2;
}

static int l_player_is_flight(lua_State* L) {
    if (auto player = get_player(L, 1)) {
        lua_pushboolean(L, player->isFlight());
        return 1;
    }
    return 0;
}

static int l_player_set_flight(lua_State* L) {
    if (auto player = get_player(L, 1)) {
        player->setFlight(lua_toboolean(L, 2));
    }
    return 0;
}

static int l_player_is_noclip(lua_State* L) {
    if (auto player = get_player(L, 1)) {
        lua_pushboolean(L, player->isNoclip());
        return 1;
    }
    return 0;
}

static int l_player_set_noclip(lua_State* L) {
    if (auto player = get_player(L, 1)) {
        player->setNoclip(lua_toboolean(L, 2));
    }
    return 0;
}

const luaL_Reg playerlib [] = {
    {"get_pos", lua_wrap_errors<l_player_get_pos>},
    {"set_pos", lua_wrap_errors<l_player_set_pos>},
    {"get_vel", lua_wrap_errors<l_player_get_vel>},
    {"set_vel", lua_wrap_errors<l_player_set_vel>},
    {"get_rot", lua_wrap_errors<l_player_get_rot>},
    {"set_rot", lua_wrap_errors<l_player_set_rot>},
    {"get_inventory", lua_wrap_errors<l_player_get_inv>},
    {"is_flight", lua_wrap_errors<l_player_is_flight>},
    {"set_flight", lua_wrap_errors<l_player_set_flight>},
    {"is_noclip", lua_wrap_errors<l_player_is_noclip>},
    {"set_noclip", lua_wrap_errors<l_player_set_noclip>},
    {NULL, NULL}
};
