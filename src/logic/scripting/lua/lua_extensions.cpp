#include <iostream>
#include <iomanip>

#include "libs/api_lua.hpp"
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
        for (int i = 0; i < depth; ++i) std::cout << "  ";
    };

    auto printHexData = [](const void* ptr, size_t size) {
        const auto* bytePtr = reinterpret_cast<const uint8_t*>(ptr);
        for (size_t i = 0; i < size; ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0')
                      << static_cast<int>(bytePtr[i])
                      << ((i + 1) % 8 == 0 && i + 1 < size ? "\n" : " ");
        }
    };

    auto printEscapedString = [](const char* str) {
        while (*str) {
            switch (*str) {
                case '\\': std::cout << "\\\\"; break;
                case '\"': std::cout << "\\\""; break;
                case '\n': std::cout << "\\n"; break;
                case '\t': std::cout << "\\t"; break;
                case '\r': std::cout << "\\r"; break;
                case '\b': std::cout << "\\b"; break;
                case '\f': std::cout << "\\f"; break;
                default:
                    if (iscntrl(static_cast<unsigned char>(*str))) {
                        // Print other control characters in \xHH format
                        std::cout << "\\x" << std::hex << std::setw(2) << std::setfill('0')
                                  << static_cast<int>(static_cast<unsigned char>(*str)) << std::dec;
                    } else {
                        std::cout << *str;
                    }
                    break;
            }
            ++str;
        }
    };

    std::function<void(int, int, bool)> debugPrint = [&](int index,
                                                         int depth,
                                                         bool is_key) {
        if (depth > MAX_DEPTH) {
            std::cout << "{...}";
            return;
        }
        switch (lua::type(L, index)) {
            case LUA_TSTRING:
                if (is_key){
                    std::cout << lua::tostring(L, index);
                }else{
                    std::cout << "\"";
                    printEscapedString(lua::tostring(L, index));
                    std::cout << "\"";
                }
                break;
            case LUA_TBOOLEAN:
                std::cout << (lua::toboolean(L, index) ? "true" : "false");
                break;
            case LUA_TNUMBER:
                std::cout << lua::tonumber(L, index);
                break;
            case LUA_TTABLE: {
                bool is_list = lua::objlen(L, index) > 0, hadItems = false;
                int absTableIndex =
                    index > 0 ? index : lua::gettop(L) + index + 1;
                std::cout << "{";
                lua::pushnil(L);
                while (lua::next(L, absTableIndex) != 0) {
                    if (hadItems)
                        std::cout << "," << '\n';
                    else
                        std::cout << '\n';

                    addIndentation(depth + 1);
                    if (!is_list) {
                        debugPrint(-2, depth, true);
                        std::cout << " = ";
                    }
                    debugPrint(-1, depth + 1, false);
                    lua::pop(L, 1);
                    hadItems = true;
                }
                if (hadItems) std::cout << '\n';
                addIndentation(depth);
                std::cout << "}";
                break;
            }
            case LUA_TFUNCTION:
                std::cout << "function(0x" << std::hex
                          << lua::topointer(L, index) << std::dec << ")";
                break;
            case LUA_TUSERDATA:
                std::cout << "userdata:\n";
                printHexData(lua::topointer(L, index), lua::objlen(L, index));
                break;
            case LUA_TLIGHTUSERDATA:
                std::cout << "lightuserdata:\n";
                printHexData(lua::topointer(L, index), sizeof(void*));
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
    std::cout << "debug.print(" << '\n';
    for (int i = 1; i <= n; ++i) {
        addIndentation(1);
        debugPrint(i, 1, false);
        if (i < n) std::cout << "," << '\n';
    }
    std::cout << '\n' << ")" << std::endl;
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
