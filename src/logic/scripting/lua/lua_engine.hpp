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
        State*,
        const std::string& name,
        std::function<int(State*)> args = [](auto*) { return 0; }
    );
    State* get_main_state();
    State* create_state(StateType stateType);
    [[nodiscard]] scriptenv create_environment(State* L);

    void init_state(State* L, StateType stateType);
}
