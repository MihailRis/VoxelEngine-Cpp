#include <glm/glm.hpp>
#include <iostream>

#include "../../../assets/Assets.hpp"
#include "../../../content/Content.hpp"
#include "../../../engine.hpp"
#include "../../../frontend/UiDocument.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../graphics/ui/elements/InventoryView.hpp"
#include "../../../items/Inventories.hpp"
#include "../../../logic/BlocksController.hpp"
#include "../../../objects/Player.hpp"
#include "../../../util/stringutil.hpp"
#include "../../../voxels/Block.hpp"
#include "../../../voxels/Chunks.hpp"
#include "../../../voxels/voxel.hpp"
#include "../../../world/Level.hpp"
#include "api_lua.hpp"

namespace scripting {
    extern Hud* hud;
}
using namespace scripting;

static int l_hud_open_inventory(lua::State*) {
    if (!hud->isInventoryOpen()) {
        hud->openInventory();
    }
    return 0;
}

static int l_hud_close_inventory(lua::State*) {
    if (hud->isInventoryOpen()) {
        hud->closeInventory();
    }
    return 0;
}

static int l_hud_open_block(lua::State* L) {
    auto x = lua::tointeger(L, 1);
    auto y = lua::tointeger(L, 2);
    auto z = lua::tointeger(L, 3);
    bool playerInventory = !lua::toboolean(L, 4);

    auto vox = level->chunks->get(x, y, z);
    if (vox == nullptr) {
        throw std::runtime_error(
            "block does not exists at " + std::to_string(x) + " " +
            std::to_string(y) + " " + std::to_string(z)
        );
    }
    auto& def = content->getIndices()->blocks.require(vox->id);
    auto assets = engine->getAssets();
    auto layout = assets->get<UiDocument>(def.uiLayout);
    if (layout == nullptr) {
        throw std::runtime_error("block '" + def.name + "' has no ui layout");
    }

    auto id = blocks->createBlockInventory(x, y, z);
    hud->openInventory(
        glm::ivec3(x, y, z),
        layout,
        level->inventories->get(id),
        playerInventory
    );

    lua::pushinteger(L, id);
    lua::pushstring(L, def.uiLayout);
    return 2;
}

static int l_hud_show_overlay(lua::State* L) {
    auto name = lua::require_string(L, 1);
    bool playerInventory = lua::toboolean(L, 2);

    auto assets = engine->getAssets();
    auto layout = assets->get<UiDocument>(name);
    if (layout == nullptr) {
        throw std::runtime_error("there is no ui layout " + util::quote(name));
    }
    hud->showOverlay(layout, playerInventory);
    return 0;
}

static UiDocument* require_layout(const char* name) {
    auto assets = engine->getAssets();
    auto layout = assets->get<UiDocument>(name);
    if (layout == nullptr) {
        throw std::runtime_error(
            "layout '" + std::string(name) + "' is not found"
        );
    }
    return layout;
}

static int l_hud_open_permanent(lua::State* L) {
    auto layout = require_layout(lua::require_string(L, 1));
    hud->openPermanent(layout);
    return 0;
}

static int l_hud_close(lua::State* L) {
    auto layout = require_layout(lua::require_string(L, 1));
    hud->remove(layout->getRoot());
    return 0;
}

static int l_hud_pause(lua::State*) {
    hud->setPause(true);
    return 0;
}

static int l_hud_resume(lua::State*) {
    hud->setPause(false);
    return 0;
}

static int l_hud_get_block_inventory(lua::State* L) {
    auto inventory = hud->getBlockInventory();
    if (inventory == nullptr) {
        return lua::pushinteger(L, 0);
    } else {
        return lua::pushinteger(L, inventory->getId());
    }
}

static int l_hud_get_player(lua::State* L) {
    auto player = hud->getPlayer();
    return lua::pushinteger(L, player->getId());
}

static int l_hud_is_paused(lua::State* L) {
    return lua::pushboolean(L, hud->isPause());
}

static int l_hud_is_inventory_open(lua::State* L) {
    return lua::pushboolean(L, hud->isInventoryOpen());
}

const luaL_Reg hudlib[] = {
    {"open_inventory", lua::wrap<l_hud_open_inventory>},
    {"close_inventory", lua::wrap<l_hud_close_inventory>},
    {"open_block", lua::wrap<l_hud_open_block>},
    {"open_permanent", lua::wrap<l_hud_open_permanent>},
    {"show_overlay", lua::wrap<l_hud_show_overlay>},
    {"get_block_inventory", lua::wrap<l_hud_get_block_inventory>},
    {"close", lua::wrap<l_hud_close>},
    {"pause", lua::wrap<l_hud_pause>},
    {"resume", lua::wrap<l_hud_resume>},
    {"is_paused", lua::wrap<l_hud_is_paused>},
    {"is_inventory_open", lua::wrap<l_hud_is_inventory_open>},
    {"get_player", lua::wrap<l_hud_get_player>},
    {NULL, NULL}};
