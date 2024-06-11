#ifndef LOGIC_SCRIPTING_LUA_HPP_
#define LOGIC_SCRIPTING_LUA_HPP_

#include "../../../delegates.hpp"
#include "../scripting.hpp"

#ifdef __linux__ 
#include <luajit-2.1/luaconf.h>
#include <luajit-2.1/lua.hpp>
#else
#include <lua.hpp>
#endif
#include <glm/glm.hpp>

#include <string>
#include <stdexcept>

#ifndef LUAJIT_VERSION
#error LuaJIT required
#endif

namespace lua {
    class luaerror : public std::runtime_error {
    public:
        luaerror(const std::string& message);
    };

    void log_error(const std::string& text);
}

#endif // LOGIC_SCRIPTING_LUA_HPP_
