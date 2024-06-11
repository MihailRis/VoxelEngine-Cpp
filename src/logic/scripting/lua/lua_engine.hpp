#ifndef LOGIC_SCRIPTING_LUA_STATE_HPP_
#define LOGIC_SCRIPTING_LUA_STATE_HPP_

#include "lua_util.hpp"

#include "../scripting_functional.hpp"
#include "../../../data/dynamic.hpp"
#include "../../../delegates.hpp"

#include <string>
#include <stdexcept>

namespace lua {
    void initialize();
    void finalize();

    bool emit_event(lua::State*, const std::string& name, std::function<int(lua::State*)> args=[](auto*){return 0;});
    lua::State* get_main_thread();
}

#endif // LOGIC_SCRIPTING_LUA_STATE_HPP_
