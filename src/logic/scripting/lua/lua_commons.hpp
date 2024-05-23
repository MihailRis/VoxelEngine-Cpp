#ifndef LOGIC_SCRIPTING_LUA_HPP_
#define LOGIC_SCRIPTING_LUA_HPP_

#ifdef __linux__ 
#include <luajit-2.1/luaconf.h>
#include <luajit-2.1/lua.hpp>
#else
#include <lua.hpp>
#endif

#ifndef LUAJIT_VERSION
#error LuaJIT required
#endif

#include <string>
#include <exception>

template <lua_CFunction func> int lua_wrap_errors(lua_State *L) {
    int result = 0;
    try {
        result = func(L);
    }
    // transform exception with description into lua_error
    catch (std::exception &e) {
        luaL_error(L, e.what());
    }
    // Rethrow any other exception (lua error for example)
    catch (...) {
        throw;
    }
    return result;
}

#endif // LOGIC_SCRIPTING_LUA_HPP_
