#include "api_lua.hpp"
#include "lua_commons.hpp"
#include "LuaState.hpp"

#include "../../../coders/toml.hpp"
#include "../../../data/dynamic.hpp"

namespace scripting {
    extern lua::LuaState* state;
}
using namespace scripting;

static int l_toml_stringify(lua_State* L) {
    auto value = state->tovalue(1);

    if (auto mapptr = std::get_if<dynamic::Map_sptr>(&value)) {
        auto string = toml::stringify(**mapptr);
        lua_pushstring(L, string.c_str());
        return 1;
    } else {
        throw std::runtime_error("table expected");
    }
}

static int l_toml_parse(lua_State*) {
    auto string = state->requireString(1);
    auto element = toml::parse("<string>", string);
    auto value = std::make_unique<dynamic::Value>(element);
    state->pushvalue(*value);
    return 1;
}

const luaL_Reg tomllib [] = {
    {"serialize", lua_wrap_errors<l_toml_stringify>},
    {"deserialize", lua_wrap_errors<l_toml_parse>},
    {NULL, NULL}
};
