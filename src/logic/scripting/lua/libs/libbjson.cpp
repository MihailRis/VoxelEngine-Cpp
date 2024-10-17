#include "coders/binary_json.hpp"
#include "api_lua.hpp"
#include "util/Buffer.hpp"
#include "../lua_custom_types.hpp"

static int l_tobytes(lua::State* L) {
    auto value = lua::tovalue(L, 1);
    bool compress = true;
    if (lua::gettop(L) >= 2) {
        compress = lua::toboolean(L, 2);
    }
    return lua::newuserdata<lua::LuaBytearray>(
        L, json::to_binary(value, compress)
    );
}

static int l_frombytes(lua::State* L) {
    if (lua::istable(L, 1)) {
        size_t len = lua::objlen(L, 1);
        util::Buffer<ubyte> buffer(len);
        for (size_t i = 0; i < len; i++) {
            lua::rawgeti(L, i + 1);
            buffer[i] = lua::tointeger(L, -1);
            lua::pop(L);
        }
        return lua::pushvalue(L, json::from_binary(buffer.data(), len));
    } else if (auto bytes = lua::touserdata<lua::LuaBytearray>(L, -1)) {
        const auto& buffer = bytes->data();
        return lua::pushvalue(
            L, json::from_binary(buffer.data(), buffer.size())
        );
    } else {
        throw std::runtime_error("table or Bytearray expected");
    }
}

const luaL_Reg bjsonlib[] = {
    {"tobytes", lua::wrap<l_tobytes>},
    {"frombytes", lua::wrap<l_frombytes>},
    {NULL, NULL}};
