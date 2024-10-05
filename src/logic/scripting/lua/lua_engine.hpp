#pragma once

#include <stdexcept>
#include <string>

#include "delegates.hpp"
#include "logic/scripting/scripting_functional.hpp"
#include "lua_util.hpp"

namespace lua {
    enum class StateType {
        BASE,
        GENERATOR,
    };

    void initialize();
    void finalize();

    bool emit_event(
        lua::State*,
        const std::string& name,
        std::function<int(lua::State*)> args = [](auto*) { return 0; }
    );
    lua::State* get_main_thread();

    void init_state(lua::State* L, StateType stateType);
}
