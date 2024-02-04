#ifndef LOGIC_SCRIPTING_LUA_UTIL_H_
#define LOGIC_SCRIPTING_LUA_UTIL_H_

#include <lua.hpp>

namespace lua {
    inline int pushivec3(lua_State* L, luaint x, luaint y, luaint z) {
        lua_pushinteger(L, x);
        lua_pushinteger(L, y);
        lua_pushinteger(L, z);
        return 3;
    }
}

#endif // LOGIC_SCRIPTING_LUA_UTIL_H_
