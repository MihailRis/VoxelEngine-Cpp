#include "api_lua.hpp"

#include <sstream>
#include <glm/glm.hpp>

template<int n, template<class> class Operation>
static int l_binop(lua::State* L) {
    uint argc = lua::gettop(L);
    if (argc != 2 && argc != 3) {
        throw std::runtime_error("invalid arguments number (2 or 3 expected)");
    }
    const auto& a = lua::tovec<n>(L, 1);

    if (lua::isnumber(L, 2)) { // scalar second operand overload
        const auto& b = lua::tonumber(L, 2);
        Operation op;
        if (argc == 2) {
            lua::createtable(L, n, 0);
            for (uint i = 0; i < n; i++) {
                lua::pushnumber(L, op(a[i], b));
                lua::rawseti(L, i+1);
            }
            return 1;
        } else {
            return lua::setvec(L, 3, op(a, glm::vec<n, float>(b)));
        }
    } else {
        const auto& b = lua::tovec<n>(L, 2);
        Operation op;
        if (argc == 2) {
            lua::createtable(L, n, 0);
            for (uint i = 0; i < n; i++) {
                lua::pushnumber(L, op(a[i], b[i]));
                lua::rawseti(L, i+1);
            }
            return 1;
        } else {
            return lua::setvec(L, 3, op(a, b));
        }
    }
}

template<int n, glm::vec<n, float>(*func)(const glm::vec<n, float>&)>
static int l_unaryop(lua::State* L) {
    uint argc = lua::gettop(L);
    const auto& vec = func(lua::tovec<n>(L, 1));
    switch (argc) {
        case 1:
            lua::createtable(L, n, 0);
            for (uint i = 0; i < n; i++) {
                lua::pushnumber(L, vec[i]);
                lua::rawseti(L, i+1);
            }
            return 1;
        case 2:
            return lua::setvec(L, 2, vec);
        default: {
            throw std::runtime_error("invalid arguments number (1 or 2 expected)");
        }
    }
}

template<int n, float(*func)(const glm::vec<n, float>&)>
static int l_scalar_op(lua::State* L) {
    if (lua::gettop(L) != 1) {
        throw std::runtime_error("invalid arguments number (1 expected)");
    }
    return lua::pushnumber(L, func(lua::tovec<n>(L, 1)/*vector a*/));
}

template<int n>
static int l_pow(lua::State* L) {
    if (lua::gettop(L) != 2) {
        throw std::runtime_error("invalid arguments number (2 expected)");
    }
    const auto& a = lua::tovec<n>(L, 1); //vector
    const auto& b = lua::tonumber(L, 2); //scalar (pow)
    glm::vec<n, float> result_vector;
    for (int i = 0; i < n; i++) {
        result_vector[i] = pow(a[i], b);
    }
    return lua::setvec(L, 1, result_vector);
}

template<int n>
static int l_dot(lua::State* L) {
    if (lua::gettop(L) != 2) {
        throw std::runtime_error("invalid arguments number (2 expected)");
    }
    return lua::pushnumber(L, glm::dot(lua::tovec<n>(L, 1), // vector a
                                       lua::tovec<n>(L, 2) // vector b
                                       ));
}

template<int n>
static int l_round(lua::State* L) {
    if (lua::gettop(L)!= 1) {
        throw std::runtime_error("invalid arguments number (1 expected)");
    }
    const auto& vec = lua::tovec<n>(L, 1);
    glm::vec<n, float> rounded_vector;
    for (int i = 0; i < n; i++) {
        rounded_vector[i] = std::round(vec[i]);
    }
    return lua::setvec(L, 1, rounded_vector);
}

template<int n>
static int l_tostring(lua::State* L) {
    const auto& vec = lua::tovec<n>(L, 1);
    if (lua::gettop(L) != 1) {
        throw std::runtime_error("invalid arguments number (1 expected)");
    }
    std::stringstream ss;
    ss << "vec" << std::to_string(n) << "{";
    for (int i = 0; i < n; i++) {
        if (i > 0) {
            ss << ", ";
        }
        ss << vec[i];
    }
    ss << "}";
    return lua::pushstring(L, ss.str());
}

const luaL_Reg vec2lib [] = {
    {"add", lua::wrap<l_binop<2, std::plus>>},
    {"sub", lua::wrap<l_binop<2, std::minus>>},
    {"mul", lua::wrap<l_binop<2, std::multiplies>>},
    {"div", lua::wrap<l_binop<2, std::divides>>},
    {"norm", lua::wrap<l_unaryop<2, glm::normalize>>},
    {"len", lua::wrap<l_scalar_op<2, glm::length>>},
    {"abs", lua::wrap<l_unaryop<2, glm::abs>>},
    {"tostring", lua::wrap<l_tostring<2>>},
    {"pow", lua::wrap<l_pow<2>>},
    {"dot", lua::wrap<l_dot<2>>},
    {"round", lua::wrap<l_round<2>>},
    {NULL, NULL}
};

const luaL_Reg vec3lib [] = {
    {"add", lua::wrap<l_binop<3, std::plus>>},
    {"sub", lua::wrap<l_binop<3, std::minus>>},
    {"mul", lua::wrap<l_binop<3, std::multiplies>>},
    {"div", lua::wrap<l_binop<3, std::divides>>},
    {"norm", lua::wrap<l_unaryop<3, glm::normalize>>},
    {"len", lua::wrap<l_scalar_op<3, glm::length>>},
    {"abs", lua::wrap<l_unaryop<3, glm::abs>>},
    {"tostring", lua::wrap<l_tostring<3>>},
    {"pow", lua::wrap<l_pow<3>>},
    {"dot", lua::wrap<l_dot<3>>},
    {"round", lua::wrap<l_round<3>>},
    {NULL, NULL}
};

const luaL_Reg vec4lib [] = {
    {"add", lua::wrap<l_binop<4, std::plus>>},
    {"sub", lua::wrap<l_binop<4, std::minus>>},
    {"mul", lua::wrap<l_binop<4, std::multiplies>>},
    {"div", lua::wrap<l_binop<4, std::divides>>},
    {"abs", lua::wrap<l_unaryop<4, glm::abs>>},
    {"norm", lua::wrap<l_unaryop<4, glm::normalize>>},
    {"len", lua::wrap<l_scalar_op<4, glm::length>>},
    {"tostring", lua::wrap<l_tostring<4>>},
    {"pow", lua::wrap<l_pow<4>>},
    {"dot", lua::wrap<l_dot<4>>},
    {"round", lua::wrap<l_round<4>>},
    {NULL, NULL}
};