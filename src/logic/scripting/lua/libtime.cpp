#include "api_lua.hpp"
#include "lua_commons.hpp"
#include "../scripting.hpp"
#include "../../../engine.hpp"
#include "../../../window/Window.hpp"

static int l_time_uptime(lua_State* L) {
    lua_pushnumber(L, Window::time());
    return 1;
}

static int l_time_delta(lua_State* L) {
    lua_pushnumber(L, scripting::engine->getDelta());
    return 1;
}

const luaL_Reg timelib [] = {
    {"uptime", lua_wrap_errors<l_time_uptime>},
    {"delta", lua_wrap_errors<l_time_delta>},
    {NULL, NULL}
};
