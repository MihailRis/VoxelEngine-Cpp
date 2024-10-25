#include "api_lua.hpp"

#include <vector>

#include "../lua_custom_types.hpp"
#include "util/stringutil.hpp"

int l_encode(lua::State* L) {
    std::string string = lua::require_string(L, 1);
    if (lua::toboolean(L, 2)) {
        lua::createtable(L, string.length(), 0);
        for (size_t i = 0; i < string.length(); i++) {
            lua::pushinteger(L, string[i]);
            lua::rawseti(L, i+1);
        }
    } else {
        lua::newuserdata<lua::LuaBytearray>(L, string.length());
        auto bytearray = lua::touserdata<lua::LuaBytearray>(L, -1);   
        bytearray->data().reserve(string.length());
        std::memcpy(bytearray->data().data(), string.data(), string.length());
    }
    return 1;
}

const luaL_Reg utf8lib[] = {
    {"encode", lua::wrap<l_encode>},
    {NULL, NULL}
};
