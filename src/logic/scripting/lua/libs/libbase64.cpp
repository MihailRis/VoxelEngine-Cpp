#include "api_lua.hpp"

#include "util/stringutil.hpp"

static int l_encode(lua::State* L) {
    if (lua::istable(L, 1)) {
        lua::pushvalue(L, 1);
        size_t size = lua::objlen(L, 1);
        util::Buffer<char> buffer(size);
        for (size_t i = 0; i < size; i++) {
            lua::rawgeti(L, i + 1);
            buffer[i] = lua::tointeger(L, -1);
            lua::pop(L);
        }
        lua::pop(L);
        return lua::pushstring(L, util::base64_encode(
            reinterpret_cast<const ubyte*>(buffer.data()), buffer.size()
        ));
    } else if (auto bytes = lua::touserdata<lua::LuaBytearray>(L, 1)) {
        return lua::pushstring(
            L,
            util::base64_encode(
                bytes->data().data(),
                bytes->data().size()
            )
        );
    }
    throw std::runtime_error("array or ByteArray expected");
}

static int l_decode(lua::State* L) {
    auto buffer = util::base64_decode(lua::require_lstring(L, 1));
    if (lua::toboolean(L, 2)) {
        lua::createtable(L, buffer.size(), 0);
        for (size_t i = 0; i < buffer.size(); i++) {
            lua::pushinteger(L, buffer[i] & 0xFF);
            lua::rawseti(L, i+1);
        }
    } else {
        lua::newuserdata<lua::LuaBytearray>(L, buffer.size());
        auto bytearray = lua::touserdata<lua::LuaBytearray>(L, -1);   
        bytearray->data().reserve(buffer.size());
        std::memcpy(bytearray->data().data(), buffer.data(), buffer.size());
    }
    return 1;
}

const luaL_Reg base64lib[] = {
    {"encode", lua::wrap<l_encode>},
    {"decode", lua::wrap<l_decode>},
    {NULL, NULL}
};
