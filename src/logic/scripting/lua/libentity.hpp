#ifndef LOGIC_SCRIPTING_LUA_LIBENTITY_HPP_
#define LOGIC_SCRIPTING_LUA_LIBENTITY_HPP_

#include "api_lua.hpp"

#include "../../LevelController.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../world/Level.hpp"
#include "../../../objects/Entities.hpp"

#include <optional>

namespace scripting {
    extern Hud* hud;
}

inline std::optional<Entity> get_entity(lua::State* L, int idx) {
    auto id = lua::tointeger(L, idx);
    auto level = scripting::controller->getLevel();
    return level->entities->get(id);
}

#endif // LOGIC_SCRIPTING_LUA_LIBENTITY_HPP_
