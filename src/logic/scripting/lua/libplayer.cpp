#include "libentity.hpp"

#include "../../../world/Level.hpp"
#include "../../../objects/Player.hpp"
#include "../../../objects/Entities.hpp"
#include "../../../physics/Hitbox.hpp"
#include "../../../window/Camera.hpp"
#include "../../../items/Inventory.hpp"

#include <glm/glm.hpp>
#include <algorithm>

using namespace scripting;

inline std::shared_ptr<Player> get_player(lua::State* L, int idx) {
    return level->getObject<Player>(lua::tointeger(L, idx));
}

static int l_get_pos(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        return lua::pushvec3_stack(L, player->getPosition());
    }
    return 0;
}

static int l_set_pos(lua::State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    auto x = lua::tonumber(L, 2);
    auto y = lua::tonumber(L, 3);
    auto z = lua::tonumber(L, 4);
    player->teleport(glm::vec3(x, y, z));
    return 0;
}

static int l_get_vel(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        if (auto hitbox = player->getHitbox()) {
            return lua::pushvec3_stack(L, hitbox->velocity);
        }
    }
    return 0;    
}

static int l_set_vel(lua::State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    auto x = lua::tonumber(L, 2);
    auto y = lua::tonumber(L, 3);
    auto z = lua::tonumber(L, 4);
    if (auto hitbox = player->getHitbox()) {
        hitbox->velocity = glm::vec3(x, y, z);
    }
    return 0;
}

static int l_get_rot(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        return lua::pushvec3_stack(L, player->cam);
    }
    return 0;
}

static int l_set_rot(lua::State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    glm::vec3& cam = player->cam;

    auto x = lua::tonumber(L, 2);
    auto y = lua::tonumber(L, 3);
    auto z = cam.z;
    if (lua::isnumber(L, 4)) {
        z = lua::tonumber(L, 4);
    }
    cam.x = x;
    cam.y = y;
    cam.z = z;
    return 0;
}

static int l_get_dir(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        return lua::pushvec3(L, player->camera->front);
    }
    return 0;
}

static int l_get_inv(lua::State* L) {
    auto player = get_player(L, 1);
    if (!player) {
        return 0;
    }
    lua::pushinteger(L, player->getInventory()->getId());
    lua::pushinteger(L, player->getChosenSlot());
    return 2;
}

static int l_is_flight(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        return lua::pushboolean(L, player->isFlight());
    }
    return 0;
}

static int l_set_flight(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        player->setFlight(lua::toboolean(L, 2));
    }
    return 0;
}

static int l_is_noclip(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        return lua::pushboolean(L, player->isNoclip());
    }
    return 0;
}

static int l_set_noclip(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        player->setNoclip(lua::toboolean(L, 2));
    }
    return 0;
}

static int l_get_selected_block(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        if (player->selection.vox.id == BLOCK_VOID) {
            return 0;
        }
        return lua::pushivec3_stack(L, player->selection.position);
    }
    return 0;
}

static int l_get_selected_entity(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        if (auto eid = player->getSelectedEntity()) {
            return lua::pushinteger(L, eid);
        }
    }
    return 0;
}

static int l_get_spawnpoint(lua::State* L) {
    if (auto player = get_player(L, 1)) {
        return lua::pushvec3_stack(L, player->getSpawnPoint());
    }
    return 0;
}


static int l_set_spawnpoint(lua::State* L) {
    auto player = get_player(L, 1);

    if (player) {
        auto x = lua::tonumber(L, 2);
        auto y = lua::tonumber(L, 3);
        auto z = lua::tonumber(L, 4);
        player->setSpawnPoint(glm::vec3(x, y, z));
    }
    return 0;
}

static int l_get_entity(lua::State* L) {
    auto player = get_player(L, 1);
    if (player == nullptr) {
        return 0;
    }
    return lua::pushinteger(L, player->getEntity());
}

static int l_set_entity(lua::State* L) {
    auto player = get_player(L, 1);
    if (player == nullptr) {
        return 0;
    }
    if (auto entity = get_entity(L, 2)) {
        player->setEntity(entity->getUID());
    }
    return 0;
}

static int l_get_camera(lua::State* L) {
    auto player = get_player(L, 1);
    if (player == nullptr) {
        return 0;
    }
    auto found = std::find(
        level->cameras.begin(), level->cameras.end(), player->currentCamera);
    if (found == level->cameras.end()) {
        return 0;
    }
    return lua::pushinteger(L, found - level->cameras.end());
}

static int l_set_camera(lua::State* L) {
    auto player = get_player(L, 1);
    if (player == nullptr) {
        return 0;
    }
    size_t index = lua::tointeger(L, 2);
    player->currentCamera = level->cameras.at(index);
    return 0;
}

const luaL_Reg playerlib [] = {
    {"get_pos", lua::wrap<l_get_pos>},
    {"set_pos", lua::wrap<l_set_pos>},
    {"get_vel", lua::wrap<l_get_vel>},
    {"set_vel", lua::wrap<l_set_vel>},
    {"get_rot", lua::wrap<l_get_rot>},
    {"set_rot", lua::wrap<l_set_rot>},
    {"get_dir", lua::wrap<l_get_dir>},
    {"get_inventory", lua::wrap<l_get_inv>},
    {"is_flight", lua::wrap<l_is_flight>},
    {"set_flight", lua::wrap<l_set_flight>},
    {"is_noclip", lua::wrap<l_is_noclip>},
    {"set_noclip", lua::wrap<l_set_noclip>},
    {"get_selected_block", lua::wrap<l_get_selected_block>},
    {"get_selected_entity", lua::wrap<l_get_selected_entity>},
    {"set_spawnpoint", lua::wrap<l_set_spawnpoint>},
    {"get_spawnpoint", lua::wrap<l_get_spawnpoint>},
    {"get_entity", lua::wrap<l_get_entity>},
    {"set_entity", lua::wrap<l_set_entity>},
    {"get_camera", lua::wrap<l_get_camera>},
    {"set_camera", lua::wrap<l_set_camera>},
    {NULL, NULL}
};
