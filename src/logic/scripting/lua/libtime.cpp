#include <engine.hpp>
#include "../../../window/Window.hpp"
#include "api_lua.hpp"

static int l_time_uptime(lua::State* L) {
    return lua::pushnumber(L, Window::time());
}

static int l_time_delta(lua::State* L) {
    return lua::pushnumber(L, scripting::engine->getDelta());
}

const luaL_Reg timelib[] = {
    {"uptime", lua::wrap<l_time_uptime>},
    {"delta", lua::wrap<l_time_delta>},
    {NULL, NULL}};
