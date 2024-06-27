#include "api_lua.hpp"

#include "../../LevelController.hpp"
#include "../../../world/Level.hpp"
#include "../../../objects/Player.hpp"
#include "../../../objects/Entities.hpp"
#include "../../../physics/Hitbox.hpp"
#include "../../../window/Camera.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../content/Content.hpp"

#include <optional>

namespace scripting {
    extern Hud* hud;
}
using namespace scripting;

static std::optional<Entity> get_entity(lua::State* L, int idx) {
    auto id = lua::tointeger(L, idx);
    auto level = controller->getLevel();
    return level->entities->get(id);
}

static int l_spawn(lua::State* L) {
    auto level = controller->getLevel();
    auto player = hud->getPlayer();
    auto defname = lua::tostring(L, 1);
    auto& def = content->entities.require(defname);
    auto id = level->entities->spawn(def, player->camera->position);
    return lua::pushinteger(L, id);
}

static int l_get_vel(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        return lua::pushvec3_arr(L, entity->getHitbox().velocity);
    }
    return 0;
}

static int l_set_vel(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->getHitbox().velocity = lua::tovec3(L, 2);
    }
    return 0;
}

const luaL_Reg entitylib [] = {
    {"spawn", lua::wrap<l_spawn>},
    {"get_vel", lua::wrap<l_get_vel>},
    {"set_vel", lua::wrap<l_set_vel>},
    {NULL, NULL}
};
