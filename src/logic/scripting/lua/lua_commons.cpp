#include "lua_commons.hpp"

#include "debug/Logger.hpp"

static debug::Logger logger("lua");

void lua::log_error(const std::string& text) {
    logger.error() << text;
}
