#include "coders/json.hpp"
#include "data/dynamic.hpp"
#include "api_lua.hpp"

static int l_json_stringify(lua::State* L) {
    auto value = lua::tovalue(L, 1);

    if (auto mapptr = std::get_if<dynamic::Map_sptr>(&value)) {
        bool nice = lua::toboolean(L, 2);
        auto string = json::stringify(mapptr->get(), nice, "  ");
        return lua::pushstring(L, string);
    } else {
        throw std::runtime_error("table expected");
    }
}

static int l_json_parse(lua::State* L) {
    auto string = lua::require_string(L, 1);
    auto element = json::parse("<string>", string);
    return lua::pushvalue(L, element);
}

const luaL_Reg jsonlib[] = {
    {"tostring", lua::wrap<l_json_stringify>},
    {"parse", lua::wrap<l_json_parse>},
    {NULL, NULL}};
