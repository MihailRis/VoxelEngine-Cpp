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

template<int n, glm::vec<n, float>(*func)(const glm::vec<n, float>&)>
static int l_unaryop(lua::State* L) {
    uint argc = lua::gettop(L);
    auto vec = func(lua::tovec<n>(L, 1));
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

template<int n, float(*func)(const glm::vec<n, float>&)>
static int l_scalar_op(lua::State* L) {
    auto vec = lua::tovec<n>(L, 1);
    if (lua::gettop(L) != 1) {
        throw std::runtime_error("invalid arguments number (1 expected)");
    }
    return lua::pushnumber(L, func(vec));
}

const luaL_Reg vec2lib [] = {
    {"add", lua::wrap<l_binop<2, std::plus>>},
    {"sub", lua::wrap<l_binop<2, std::minus>>},
    {"mul", lua::wrap<l_binop<2, std::multiplies>>},
    {"div", lua::wrap<l_binop<2, std::divides>>},
    {"normalize", lua::wrap<l_unaryop<2, glm::normalize>>},
    {"length", lua::wrap<l_scalar_op<2, glm::length>>},
    {NULL, NULL}
};

const luaL_Reg vec3lib [] = {
    {"add", lua::wrap<l_binop<3, std::plus>>},
    {"sub", lua::wrap<l_binop<3, std::minus>>},
    {"mul", lua::wrap<l_binop<3, std::multiplies>>},
    {"div", lua::wrap<l_binop<3, std::divides>>},
    {"normalize", lua::wrap<l_unaryop<3, glm::normalize>>},
    {"length", lua::wrap<l_scalar_op<3, glm::length>>},
    {NULL, NULL}
};

const luaL_Reg vec4lib [] = {
    {"add", lua::wrap<l_binop<4, std::plus>>},
    {"sub", lua::wrap<l_binop<4, std::minus>>},
    {"mul", lua::wrap<l_binop<4, std::multiplies>>},
    {"div", lua::wrap<l_binop<4, std::divides>>},
    {"normalize", lua::wrap<l_unaryop<4, glm::normalize>>},
    {"length", lua::wrap<l_scalar_op<4, glm::length>>},
    {NULL, NULL}
};
