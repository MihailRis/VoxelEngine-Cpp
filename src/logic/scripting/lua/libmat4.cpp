#include "api_lua.hpp"

#include <sstream>
#include <glm/ext/matrix_transform.hpp>

static int l_idt(lua::State* L) {
    return lua::pushmat4(L, glm::mat4(1.0f));
}

/// @brief  mat4.scale(matrix=idt: array[16], scale: array[3]) -> array[16]
/// Modifies matrix
static int l_scale(lua::State* L) {
    uint argc = lua::gettop(L);
    switch (argc) {
        case 1: {
            auto scale = lua::tovec3(L, 1);
            return lua::pushmat4(L, glm::scale(glm::mat4(1.0f), scale));
        }
        case 2: {
            auto matrix = lua::tomat4(L, 1);
            auto scale = lua::tovec3(L, 2);
            return lua::pushmat4(L, glm::scale(matrix, scale));
        }
        case 3: {
            auto matrix = lua::tomat4(L, 1);
            auto scale = lua::tovec3(L, 2);
            return lua::setmat4(L, 3, glm::scale(matrix, scale));
        }
        default: {
            throw std::runtime_error("invalid number of arguments (1 or 2 expected)");
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
    {"scale", lua::wrap<l_scale>},
    {"tostring", lua::wrap<l_tostring>},
    {NULL, NULL}
};

