#pragma once

#include <stdexcept>
#include <string>

#include "data/dynamic.hpp"
#include "delegates.hpp"
#include "logic/scripting/scripting_functional.hpp"
#include "lua_util.hpp"

namespace lua {
    void initialize();
    void finalize();

    bool emit_event(
        lua::State*,
        const std::string& name,
        std::function<int(lua::State*)> args = [](auto*) { return 0; }
    );
    lua::State* get_main_thread();
}
