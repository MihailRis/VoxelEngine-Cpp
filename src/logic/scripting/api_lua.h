#ifndef LOGIC_SCRIPTING_API_LUA_H_
#define LOGIC_SCRIPTING_API_LUA_H_

#include <lua.hpp>

#ifndef LUAJIT_VERSION
#pragma message("better use LuaJIT instead of plain Lua")
#endif // LUAJIT_VERSION

namespace apilua {
    extern void create_funcs(lua_State* L);
}

#endif // LOGIC_SCRIPTING_API_LUA_H_
