#include "api_lua.hpp"

#include <sstream>
#include <glm/ext/matrix_transform.hpp>

static int l_idt(lua::State* L) {
    return lua::pushmat4(L, glm::mat4(1.0f));
}

static int l_mul(lua::State* L) {
    uint argc = lua::gettop(L);
    if (argc < 2 || argc > 3) {
        throw std::runtime_error("invalid arguments number (2 or 3 expected)");
    }
    auto matrix1 = lua::tomat4(L, 1);
    uint len2 = lua::objlen(L, 2);
    if (len2 < 3) {
        throw std::runtime_error("argument #2: vec3 or vec4 expected");
    }
    switch (argc) {
        case 2: {
            if (len2 == 4) {
               return lua::pushvec4(L, matrix1 * lua::tovec4(L, 2));
            } else if (len2 == 3) {
               return lua::pushvec3(L, matrix1 * glm::vec4(lua::tovec3(L, 2), 1.0f));
            }
            return lua::pushmat4(L, matrix1 * lua::tomat4(L, 2));
        }
        case 3: {
            if (len2 == 4) {
               return lua::setvec4(L, 3, matrix1 * lua::tovec4(L, 2));
            } else if (len2 == 3) {
               return lua::setvec3(L, 3, matrix1 * glm::vec4(lua::tovec3(L, 2), 1.0f));
            }
            return lua::setmat4(L, 3, matrix1 * lua::tomat4(L, 2));
        }
    }
    return 0;
}

/// Overloads:
/// mat4.<func>(vec: float[3]) -> float[16] - creates transform matrix
/// mat4.<func>(matrix: float[16], vec: float[3]) -> float[16] - creates transformed copy of matrix
/// mat4.<func>(matrix: float[16], vec: float[3], dst: float[16]) -> sets dst to transformed version of matrix
template<glm::mat4(*func)(const glm::mat4&, const glm::vec3&)>
inline int l_transform_func(lua::State* L) {
    uint argc = lua::gettop(L);
    switch (argc) {
        case 1: {
            auto vec = lua::tovec3(L, 1);
            return lua::pushmat4(L, func(glm::mat4(1.0f), vec));
        }
        case 2: {
            auto matrix = lua::tomat4(L, 1);
            auto vec = lua::tovec3(L, 2);
            return lua::pushmat4(L, func(matrix, vec));
        }
        case 3: {
            auto matrix = lua::tomat4(L, 1);
            auto vec = lua::tovec3(L, 2);
            return lua::setmat4(L, 3, func(matrix, vec));
        }
        default: {
            throw std::runtime_error("invalid arguments number (1, 2 or 3 expected)");
        }
    }
    return 0;
}

/// Overloads:
/// mat4.rotate(vec: float[3], angle: float) -> float[16] - creates rotation matrix
/// mat4.rotate(matrix: float[16], vec: float[3], angle: float) -> float[16] - creates rotated copy of matrix
/// mat4.rotate(matrix: float[16], vec: float[3], angle: float, dst: float[16]) -> sets dst to rotated version of matrix
inline int l_rotate(lua::State* L) {
    uint argc = lua::gettop(L);
    switch (argc) {
        case 2: {
            auto vec = lua::tovec3(L, 1);
            auto angle = static_cast<float>(lua::tonumber(L, 2));
            return lua::pushmat4(L, glm::rotate(glm::mat4(1.0f), angle, vec));
        }
        case 3: {
            auto matrix = lua::tomat4(L, 1);
            auto vec = lua::tovec3(L, 2);
            auto angle = static_cast<float>(lua::tonumber(L, 3));
            return lua::pushmat4(L, glm::rotate(matrix, angle, vec));
        }
        case 4: {
            auto matrix = lua::tomat4(L, 1);
            auto vec = lua::tovec3(L, 2);
            auto angle = static_cast<float>(lua::tonumber(L, 3));
            return lua::setmat4(L, 3, glm::rotate(matrix, angle, vec));
        }
        default: {
            throw std::runtime_error("invalid arguments number (2, 3 or 4 expected)");
        }
    }
    return 0;
}

static int l_tostring(lua::State* L) {
    auto matrix = lua::tomat4(L, 1);
    std::stringstream ss;
    ss << "mat4 {\n";
    for (uint y = 0; y < 4; y++) {
        for (uint x = 0; x < 4; x++) {
            ss << "\t" << matrix[y][x];
        }
        ss << "\n";
    }
    ss << "}";
    return lua::pushstring(L, ss.str());
}

const luaL_Reg mat4lib [] = {
    {"idt", lua::wrap<l_idt>},
    {"mul", lua::wrap<l_mul>},
    {"scale", lua::wrap<l_transform_func<glm::scale>>},
    {"rotate", lua::wrap<l_rotate>},
    {"translate", lua::wrap<l_transform_func<glm::translate>>},
    {"tostring", lua::wrap<l_tostring>},
    {NULL, NULL}
};

