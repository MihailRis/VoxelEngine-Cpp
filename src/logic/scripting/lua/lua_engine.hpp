#pragma once

#include <stdexcept>
#include <string>

#include "delegates.hpp"
#include "logic/scripting/scripting_functional.hpp"
#include "lua_util.hpp"

class EnginePaths;
struct CoreParameters;

namespace lua {
    enum class StateType {
        BASE,
        SCRIPT,
        GENERATOR,
    };

    void initialize(const EnginePaths& paths, const CoreParameters& params);
    void finalize();

    bool emit_event(
        State*,
        const std::string& name,
        std::function<int(State*)> args = [](auto*) { return 0; }
    );
    State* get_main_state();
    State* create_state(const EnginePaths& paths, StateType stateType);
    [[nodiscard]] scriptenv create_environment(State* L);

    void init_state(State* L, StateType stateType);
}
