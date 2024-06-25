#include "api_lua.hpp"

#include <glm/glm.hpp>

template<
    uint n, 
    glm::vec<n, float>(*tofunc)(lua::State*, int),
    int(*setfunc)(lua::State*, int, glm::vec<n, float>)
>
static int l_add(lua::State* L) {
    uint argc = lua::gettop(L);
    auto a = tofunc(L, 1);
    auto b = tofunc(L, 2);
    switch (argc) {
        case 2:
            lua::createtable(L, n, 0);
            for (uint i = 0; i < n; i++) {
                lua::pushnumber(L, a[i]+b[i]);
                lua::rawseti(L, i+1);
            }
            return 1;
        case 3:
            return setfunc(L, 3, a + b);
        default: {
            throw std::runtime_error("invalid arguments number (2 or 3 expected)");
        }
    }
}

const luaL_Reg vec2lib [] = {
    {"add", lua::wrap<l_add<2, lua::tovec2, lua::setvec<2>>>},
    {NULL, NULL}
};

const luaL_Reg vec3lib [] = {
    {"add", lua::wrap<l_add<3, lua::tovec3, lua::setvec<3>>>},
    {NULL, NULL}
};

const luaL_Reg vec4lib [] = {
    {"add", lua::wrap<l_add<4, lua::tovec4, lua::setvec<4>>>},
    {NULL, NULL}
};
