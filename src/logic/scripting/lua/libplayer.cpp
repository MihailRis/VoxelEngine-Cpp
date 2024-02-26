#include "libplayer.h"
#include "../scripting.h"
#include "../../../world/Level.h"
#include "../../../objects/Player.h"
#include "../../../physics/Hitbox.h"
#include "../../../window/Camera.h"
#include "../../../items/Inventory.h"

#include <glm/glm.hpp>

/* == player library ==*/
int l_player_get_pos(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    glm::vec3 pos = scripting::level->player->hitbox->position;
    lua_pushnumber(L, pos.x);
    lua_pushnumber(L, pos.y);
    lua_pushnumber(L, pos.z);
    return 3;
}

int l_player_set_pos(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    lua::luanumber z = lua_tonumber(L, 4);
    scripting::level->player->hitbox->position = glm::vec3(x, y, z);
    return 0;
}

int l_player_get_vel(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    glm::vec3 vel = scripting::level->player->hitbox->velocity;
    lua_pushnumber(L, vel.x);
    lua_pushnumber(L, vel.y);
    lua_pushnumber(L, vel.z);
    return 3;
}

int l_player_set_vel(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    lua::luanumber z = lua_tonumber(L, 4);
    scripting::level->player->hitbox->velocity = glm::vec3(x, y, z);
    return 0;
}

int l_player_get_rot(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    glm::vec2 rot = scripting::level->player->cam;
    lua_pushnumber(L, rot.x);
    lua_pushnumber(L, rot.y);
    return 2;
}

int l_player_set_rot(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    lua::luanumber x = lua_tonumber(L, 2);
    lua::luanumber y = lua_tonumber(L, 3);
    glm::vec2& cam = scripting::level->player->cam;
    cam.x = x;
    cam.y = y;
    return 0;
}

int l_player_get_inv(lua_State* L) {
    int playerid = lua_tointeger(L, 1);
    if (playerid != 1)
        return 0;
    auto player = scripting::level->player;
    lua_pushinteger(L, player->getInventory()->getId());
    lua_pushinteger(L, player->getChosenSlot());
    return 2;
}
