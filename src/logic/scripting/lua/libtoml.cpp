#include "../../../coders/toml.hpp"
#include "../../../data/dynamic.hpp"
#include "api_lua.hpp"

using namespace scripting;

static int l_toml_stringify(lua::State* L) {
    auto value = lua::tovalue(L, 1);

    if (auto mapptr = std::get_if<dynamic::Map_sptr>(&value)) {
        auto string = toml::stringify(**mapptr);
        return lua::pushstring(L, string);
    } else {
        throw std::runtime_error("table expected");
    }
}

static int l_toml_parse(lua::State* L) {
    auto string = lua::require_string(L, 1);
    auto element = toml::parse("<string>", string);
    auto value = std::make_unique<dynamic::Value>(element);
    return lua::pushvalue(L, *value);
}

const luaL_Reg tomllib[] = {
    {"tostring", lua::wrap<l_toml_stringify>},
    {"parse", lua::wrap<l_toml_parse>},
    {NULL, NULL}};
