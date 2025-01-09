#include "engine/Engine.hpp"
#include "api_lua.hpp"

using namespace scripting;

static int l_uptime(lua::State* L) {
    return lua::pushnumber(L, engine->getTime().getTime());
}

static int l_delta(lua::State* L) {
    return lua::pushnumber(L, engine->getTime().getDelta());
}

const luaL_Reg timelib[] = {
    {"uptime", lua::wrap<l_uptime>},
    {"delta", lua::wrap<l_delta>},
    {NULL, NULL}
};
