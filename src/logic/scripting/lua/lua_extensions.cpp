#include <debug/Logger.hpp>
#include "api_lua.hpp"

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

void initialize_libs_extends(lua::State* L) {
    if (lua::getglobal(L, "debug")) {
        lua::pushcfunction(L, lua::wrap<l_debug_error>);
        lua::setfield(L, "error");

        lua::pushcfunction(L, lua::wrap<l_debug_warning>);
        lua::setfield(L, "warning");

        lua::pushcfunction(L, lua::wrap<l_debug_log>);
        lua::setfield(L, "log");

        lua::pop(L);
    }
}
