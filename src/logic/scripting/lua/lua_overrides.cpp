#include "api_lua.hpp"

#include <iostream>

/// @brief Modified version of luaB_print from lbaselib.c
int l_print(lua::State* L) {
    int n = lua::gettop(L);  /* number of arguments */
    lua::getglobal(L, "tostring");
    for (int i=1; i<=n; i++) {
        lua::pushvalue(L, -1);  /* function to be called */
        lua::pushvalue(L, i);   /* value to print */
        lua::call(L, 1, 1);
        const char* s = lua::tostring(L, -1);  /* get result */
        if (s == NULL)
            return luaL_error(L, LUA_QL("tostring") " must return a string to "
                                 LUA_QL("print"));
        if (i > 1)
            std::cout << "\t";
        std::cout << s;
        lua::pop(L);  /* pop result */
    }
    std::cout << std::endl;
    return 0;
}

