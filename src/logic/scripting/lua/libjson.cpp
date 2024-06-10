#include "api_lua.hpp"
#include "lua_util.hpp"
#include "lua_commons.hpp"
#include "LuaState.hpp"

#include "../../../coders/json.hpp"
#include "../../../data/dynamic.hpp"

namespace scripting {
    extern lua::LuaState* state;
}

static int l_json_stringify(lua_State* L) {
    auto value = lua::tovalue(L, 1);

    if (auto mapptr = std::get_if<dynamic::Map_sptr>(&value)) {
        bool nice = lua_toboolean(L, 2);
        auto string = json::stringify(mapptr->get(), nice, "  ");
        lua_pushstring(L, string.c_str());
        return 1;
    } else {
        throw std::runtime_error("table expected");
    }
}

static int l_json_parse(lua_State* L) {
    auto string = lua::require_string(L, 1);
    auto element = json::parse("<string>", string);
    lua::pushvalue(L, element);
    return 1;
}

const luaL_Reg jsonlib [] = {
    {"tostring", lua_wrap_errors<l_json_stringify>},
    {"parse", lua_wrap_errors<l_json_parse>},
    {NULL, NULL}
};
