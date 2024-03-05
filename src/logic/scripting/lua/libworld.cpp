#include "lua_commons.h"
#include "api_lua.h"
#include "../scripting.h"
#include "../../../world/Level.h"
#include "../../../world/World.h"

#include <cmath>

static int l_world_get_total_time(lua_State* L) {
    lua_pushnumber(L, scripting::level->world->totalTime);
    return 1;
}

static int l_world_get_day_time(lua_State* L) {
    lua_pushnumber(L, scripting::level->world->daytime);
    return 1;
}

static int l_world_set_day_time(lua_State* L) {
    double value = lua_tonumber(L, 1);
    scripting::level->world->daytime = fmod(value, 1.0);
    return 0;
}

static int l_world_get_seed(lua_State* L) {
    lua_pushinteger(L, scripting::level->world->getSeed());
    return 1;
}

const luaL_Reg worldlib [] = {
    {"get_total_time", lua_wrap_errors<l_world_get_total_time>},
    {"get_day_time", lua_wrap_errors<l_world_get_day_time>},
    {"set_day_time", lua_wrap_errors<l_world_set_day_time>},
    {"get_seed", lua_wrap_errors<l_world_get_seed>},
    {NULL, NULL}
};
