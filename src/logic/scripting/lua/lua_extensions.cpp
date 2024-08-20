#include <iostream>

#include "api_lua.hpp"
#include "debug/Logger.hpp"

static debug::Logger logger("lua-debug");

static int l_debug_error(lua::State* L) {
    auto text = lua::require_string(L, 1);
    logger.error() << text;
    return 0;
}

static int l_debug_warning(lua::State* L) {
    auto text = lua::require_string(L, 1);
    logger.warning() << text;
    return 0;
}

static int l_debug_log(lua::State* L) {
    auto text = lua::require_string(L, 1);
    logger.info() << text;
    return 0;
}

const int MAX_DEPTH = 10;

int l_debug_print(lua::State* L) {
    auto addIndentation = [](int depth) {
        std::cout << std::string(depth * 2, ' ');
    };

    auto pointerToHexString = [](const void* ptr, size_t size) {
        std::stringstream ss;
        const auto* bytePtr = reinterpret_cast<const uint8_t*>(ptr);
        for (size_t i = 0; i < size; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(bytePtr[i])
               << ((i + 1) % 8 == 0 && i + 1 < size ? "\n" : " ");
        }
        return ss.str();
    };

    std::function<void(int, int, bool)> debugPrint = [&](int index, int depth, bool is_key) {
        if (depth > MAX_DEPTH) {
            std::cout << "{...}";
            return;
        }
        switch (lua::type(L, index)) {
            case LUA_TSTRING:
                std::cout << (is_key ? lua::tostring(L, index) : "\"" + std::string(lua::tostring(L, index)) + "\"");
                break;
            case LUA_TBOOLEAN:
                std::cout << (lua::toboolean(L, index) ? "true" : "false");
                break;
            case LUA_TNUMBER:
                std::cout << lua::tonumber(L, index);
                break;
            case LUA_TTABLE: {
                bool is_list = lua::objlen(L, index) > 0, hadItems = false;
                int absTableIndex = index > 0 ? index : lua::gettop(L) + index + 1;
                std::cout << "{";
                lua::pushnil(L);
                while (lua::next(L, absTableIndex) != 0) {
                    if (hadItems) std::cout << "," << std::endl;
                    else std::cout << std::endl;
                    addIndentation(depth + 1);
                    if (!is_list) debugPrint(-2, depth, true), std::cout << " = ";
                    debugPrint(-1, depth + 1, false);
                    lua::pop(L, 1);
                    hadItems = true;
                }
                if (hadItems) std::cout << std::endl;
                addIndentation(depth);
                std::cout << "}";
                break;
            }
            case LUA_TFUNCTION:
                std::cout << "function(0x" << std::hex << lua::topointer(L, index) << std::dec << ")";
                break;
            case LUA_TUSERDATA:
                std::cout << "userdata:\n" << pointerToHexString(lua::topointer(L, index), lua::objlen(L, index));
                break;
            case LUA_TLIGHTUSERDATA:
                std::cout << "lightuserdata:\n" << pointerToHexString(lua::topointer(L, index), sizeof(void*));
                break;
            case LUA_TNIL:
                std::cout << "nil";
                break;
            default:
                std::cout << lua::type_name(L, lua::type(L, index));
                break;
        }
    };

    int n = lua::gettop(L);
    std::cout << "debug.print(" << std::endl;
    for (int i = 1; i <= n; ++i) {
        addIndentation(1);
        debugPrint(i, 1, false);
        if (i < n) std::cout << "," << std::endl;
    }
    std::cout << std::endl << ")" << std::endl;
    lua::pop(L, n);
    return 0;
}



void initialize_libs_extends(lua::State* L) {
    if (lua::getglobal(L, "debug")) {
        lua::pushcfunction(L, lua::wrap<l_debug_error>);
        lua::setfield(L, "error");

        lua::pushcfunction(L, lua::wrap<l_debug_warning>);
        lua::setfield(L, "warning");

        lua::pushcfunction(L, lua::wrap<l_debug_log>);
        lua::setfield(L, "log");

        lua::pushcfunction(L, lua::wrap<l_debug_print>);
        lua::setfield(L, "print");

        lua::pop(L);
    }
}
