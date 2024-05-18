#include "lua_commons.hpp"
#include "api_lua.hpp"
#include "../scripting.hpp"
#include "../../../world/Level.hpp"
#include "../../../objects/Player.hpp"
#include "../../../physics/Hitbox.hpp"
#include "../../../window/Camera.hpp"
#include "../../../items/Inventory.hpp"

#include <glm/glm.hpp>

static int l_player_get_pos(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    auto player = scripting::level->getObject<Player>(playerid);
    if (!player) return 0;
    glm::vec3 pos = player->hitbox->position;
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

static int l_player_set_pos(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    lua::luanumber z = lua_tonumber(L, 4);
    auto player = scripting::level->getObject<Player>(playerid);
    if (player) player->hitbox->position = glm::vec3(x, y, z);
    return 0;
}

static int l_player_get_vel(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    auto player = scripting::level->getObject<Player>(playerid);
    if (!player) return 0;
    glm::vec3 vel = player->hitbox->velocity;
    lua_pushnumber(L, vel.x);
    lua_pushnumber(L, vel.y);
    lua_pushnumber(L, vel.z);
    return 3;
}

static int l_player_set_vel(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    lua::luanumber z = lua_tonumber(L, 4);
    auto player = scripting::level->getObject<Player>(playerid);
    if (player) player->hitbox->velocity = glm::vec3(x, y, z);
    return 0;
}

static int l_player_get_rot(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    auto player = scripting::level->getObject<Player>(playerid);
    if (!player) return 0;
    glm::vec2 rot = player->cam;
    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    return 2;
}

static int l_player_set_rot(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    auto player = scripting::level->getObject<Player>(playerid);
    if (!player) return 0;
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    glm::vec2& cam = player->cam;
    cam.x = x;
    cam.y = y;
    return 0;
}

static int l_player_get_inv(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    auto player = scripting::level->getObject<Player>(playerid);
    if (!player) return 0;
    lua_pushinteger(L, player->getInventory()->getId());
    lua_pushinteger(L, player->getChosenSlot());
    return 2;
}

const luaL_Reg playerlib [] = {
    {"get_pos", lua_wrap_errors<l_player_get_pos>},
    {"set_pos", lua_wrap_errors<l_player_set_pos>},
    {"get_vel", lua_wrap_errors<l_player_get_vel>},
    {"set_vel", lua_wrap_errors<l_player_set_vel>},
    {"get_rot", lua_wrap_errors<l_player_get_rot>},
    {"set_rot", lua_wrap_errors<l_player_set_rot>},
    {"get_inventory", lua_wrap_errors<l_player_get_inv>},
    {NULL, NULL}
};
