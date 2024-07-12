#include "libentity.hpp"

#include "../../../objects/Player.hpp"
#include "../../../physics/Hitbox.hpp"
#include "../../../window/Camera.hpp"
#include "../../../content/Content.hpp"
#include "../../../engine.hpp"

using namespace scripting;

static int l_exists(lua::State* L) {
    return lua::pushboolean(L, get_entity(L, 1).has_value());
}

static int l_spawn(lua::State* L) {
    auto level = controller->getLevel();
    auto defname = lua::tostring(L, 1);
    auto& def = content->entities.require(defname);
    auto pos = lua::tovec3(L, 2);
    dynamic::Value args = dynamic::NONE;
    if (lua::gettop(L) > 2) {
        args = lua::tovalue(L, 3);
    }
    level->entities->spawn(def, pos, args);
    return 1;
}

static int l_despawn(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        entity->destroy();
    }
    return 0;
}

static int l_set_skeleton(lua::State* L) {
    if (auto entity = get_entity(L, 1)) {
        std::string skeletonName = lua::require_string(L, 2);
        auto rigConfig = content->getRig(skeletonName);
        if (rigConfig == nullptr) {
            throw std::runtime_error("skeleton not found '"+skeletonName+"'");
        }
        entity->setRig(rigConfig);
    }
    return 0;
}

const luaL_Reg entitylib [] = {
    {"exists", lua::wrap<l_exists>},
    {"spawn", lua::wrap<l_spawn>},
    {"despawn", lua::wrap<l_despawn>},
    {"set_skeleton", lua::wrap<l_set_skeleton>},
    {NULL, NULL}
};
