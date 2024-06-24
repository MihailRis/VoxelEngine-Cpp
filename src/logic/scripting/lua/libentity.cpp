#include "api_lua.hpp"

#include "../../LevelController.hpp"
#include "../../../world/Level.hpp"
#include "../../../objects/Player.hpp"
#include "../../../objects/Entities.hpp"
#include "../../../physics/Hitbox.hpp"
#include "../../../window/Camera.hpp"
#include "../../../frontend/hud.hpp"

namespace scripting {
    extern Hud* hud;
}
using namespace scripting;

static int l_test(lua::State* L) {
    auto level = controller->getLevel();
    auto player = hud->getPlayer();
    level->entities->drop(player->camera->position, player->camera->front*8.0f+glm::vec3(0, 2, 0)+player->hitbox->velocity);
    return 0;        
}

const luaL_Reg entitylib [] = {
    {"test", lua::wrap<l_test>},
    {NULL, NULL}
};
