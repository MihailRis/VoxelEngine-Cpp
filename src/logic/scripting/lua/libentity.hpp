#ifndef LOGIC_SCRIPTING_LUA_LIBENTITY_HPP_
#define LOGIC_SCRIPTING_LUA_LIBENTITY_HPP_

#include <optional>

#include "../../../frontend/hud.hpp"
#include <objects/Entities.hpp>
#include <world/Level.hpp>
#include <logic/LevelController.hpp>
#include "api_lua.hpp"

namespace scripting {
    extern Hud* hud;
}

inline std::optional<Entity> get_entity(lua::State* L, int idx) {
    auto id = lua::tointeger(L, idx);
    auto level = scripting::controller->getLevel();
    return level->entities->get(id);
}

#endif  // LOGIC_SCRIPTING_LUA_LIBENTITY_HPP_
