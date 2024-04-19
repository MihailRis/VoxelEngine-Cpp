#include "lua_commons.h"
#include "api_lua.h"
#include "../scripting.h"
#include "../../../world/Level.h"
#include "../../../world/World.h"
#include "../../../engine.h"

#include <cmath>
#include <filesystem>

namespace fs = std::filesystem;

static int l_world_get_total_time(lua_State* L) {
    lua_pushnumber(L, scripting::level->getWorld()->totalTime);
    return 1;
}

static int l_world_get_day_time(lua_State* L) {
    lua_pushnumber(L, scripting::level->getWorld()->daytime);
    return 1;
}

static int l_world_set_day_time(lua_State* L) {
    double value = lua_tonumber(L, 1);
    scripting::level->getWorld()->daytime = fmod(value, 1.0);
    return 0;
}

static int l_world_get_seed(lua_State* L) {
    lua_pushinteger(L, scripting::level->getWorld()->getSeed());
    return 1;
}

static int l_world_exists(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto worldsDir = scripting::engine->getPaths()->getWorldFolder(name);
    lua_pushboolean(L, fs::is_directory(worldsDir));
    return 1;
}

const luaL_Reg worldlib [] = {
    {"get_total_time", lua_wrap_errors<l_world_get_total_time>},
    {"get_day_time", lua_wrap_errors<l_world_get_day_time>},
    {"set_day_time", lua_wrap_errors<l_world_set_day_time>},
    {"get_seed", lua_wrap_errors<l_world_get_seed>},
    {"exists", lua_wrap_errors<l_world_exists>},
    {NULL, NULL}
};
