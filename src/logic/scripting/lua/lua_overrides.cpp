#include "api_lua.hpp"
#include "lua_commons.hpp"

#include <iostream>

/// @brief Modified version of luaB_print from lbaselib.c
int l_print(lua_State* L) {
    int n = lua_gettop(L);  /* number of arguments */
    lua_getglobal(L, "tostring");
    for (int i=1; i<=n; i++) {
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        const char* s = lua_tostring(L, -1);  /* get result */
        if (s == NULL)
            return luaL_error(L, LUA_QL("tostring") " must return a string to "
                                 LUA_QL("print"));
        if (i > 1)
            std::cout << "\t";
        std::cout << s;
        lua_pop(L, 1);  /* pop result */
    }
    std::cout << std::endl;
    return 0;
}

