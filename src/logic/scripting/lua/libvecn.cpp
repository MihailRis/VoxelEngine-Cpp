#include "api_lua.hpp"

#include <glm/glm.hpp>

template<int n, template<class> class Op>
static int l_binop(lua::State* L) {
    uint argc = lua::gettop(L);
    auto a = lua::tovec<n>(L, 1);
    auto b = lua::tovec<n>(L, 2);
    Op op;
    switch (argc) {
        case 2:
            lua::createtable(L, n, 0);
            for (uint i = 0; i < n; i++) {
                lua::pushnumber(L, op(a[i], b[i]));
                lua::rawseti(L, i+1);
            }
            return 1;
        case 3:
            return lua::setvec(L, 3, op(a, b));
        default: {
            throw std::runtime_error("invalid arguments number (2 or 3 expected)");
        }
    }
}

template<int n>
static int l_normalize(lua::State* L) {
    uint argc = lua::gettop(L);
    auto vec = glm::normalize(lua::tovec<n>(L, 1));
    switch (argc) {
        case 1:
            lua::createtable(L, n, 0);
            for (uint i = 0; i < n; i++) {
                lua::pushnumber(L, vec[i]);
                lua::rawseti(L, i+1);
            }
            return 1;
        case 3:
            return lua::setvec(L, 3, vec);
        default: {
            throw std::runtime_error("invalid arguments number (1 or 2 expected)");
        }
    }
}

template<int n>
static int l_len(lua::State* L) {
    auto vec = lua::tovec<n>(L, 1);
    if (lua::gettop(L) != 1) {
        throw std::runtime_error("invalid arguments number (1 expected)");
    }
    return lua::pushnumber(L, glm::length(vec));
}

const luaL_Reg vec2lib [] = {
    {"add", lua::wrap<l_binop<2, std::plus>>},
    {"sub", lua::wrap<l_binop<2, std::minus>>},
    {"mul", lua::wrap<l_binop<2, std::multiplies>>},
    {"div", lua::wrap<l_binop<2, std::divides>>},
    {"normalize", lua::wrap<l_normalize<2>>},
    {"length", lua::wrap<l_len<2>>},
    {NULL, NULL}
};

const luaL_Reg vec3lib [] = {
    {"add", lua::wrap<l_binop<3, std::plus>>},
    {"sub", lua::wrap<l_binop<3, std::minus>>},
    {"mul", lua::wrap<l_binop<3, std::multiplies>>},
    {"div", lua::wrap<l_binop<3, std::divides>>},
    {"normalize", lua::wrap<l_normalize<3>>},
    {"length", lua::wrap<l_len<3>>},
    {NULL, NULL}
};

const luaL_Reg vec4lib [] = {
    {"add", lua::wrap<l_binop<4, std::plus>>},
    {"sub", lua::wrap<l_binop<4, std::minus>>},
    {"mul", lua::wrap<l_binop<4, std::multiplies>>},
    {"div", lua::wrap<l_binop<4, std::divides>>},
    {"normalize", lua::wrap<l_normalize<4>>},
    {"length", lua::wrap<l_len<4>>},
    {NULL, NULL}
};
