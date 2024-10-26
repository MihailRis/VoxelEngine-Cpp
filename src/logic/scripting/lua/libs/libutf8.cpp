#include "api_lua.hpp"

#include <vector>

#include "../lua_custom_types.hpp"
#include "util/stringutil.hpp"

static int l_encode(lua::State* L) {
    std::string_view string = lua::require_string(L, 1);
    if (lua::toboolean(L, 2)) {
        lua::createtable(L, string.length(), 0);
        for (size_t i = 0; i < string.length(); i++) {
            lua::pushinteger(L, string[i] & 0xFF);
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

static int l_decode(lua::State* L) {
    if (lua::istable(L, 1)) {
        size_t size = lua::objlen(L, 1);
        util::Buffer<char> buffer(size);
        return lua::pushstring(L, std::string(buffer.data(), size));
    } else if (auto bytes = lua::touserdata<lua::LuaBytearray>(L, 1)) {
        return lua::pushstring(
            L,
            std::string(
                reinterpret_cast<char*>(bytes->data().data()),
                bytes->data().size()
            )
        );
    }
    return 1;
}

static int l_length(lua::State* L) {
    auto string = lua::require_string(L, 1);
    return lua::pushinteger(L, util::length_utf8(string));
}

static int l_codepoint(lua::State* L) {
    std::string_view string = lua::require_string(L, 1);
    if (string.empty()) {
        return lua::pushinteger(L, 0);
    }
    uint size;
    return lua::pushinteger(L, util::decode_utf8(size, string.data()));
}

const luaL_Reg utf8lib[] = {
    {"tobytes", lua::wrap<l_encode>},
    {"tostring", lua::wrap<l_decode>},
    {"length", lua::wrap<l_length>},
    {"codepoint", lua::wrap<l_codepoint>},
    {NULL, NULL}
};
