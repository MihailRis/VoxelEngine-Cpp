#include "api_lua.hpp"

#include "../../../assets/Assets.hpp"
#include "../../../content/Content.hpp"
#include "../../../engine.hpp"
#include "../../../frontend/hud.hpp"
#include "../../../frontend/UiDocument.hpp"
#include "../../../graphics/ui/elements/InventoryView.hpp"
#include "../../../items/Inventories.hpp"
#include "../../../logic/BlocksController.hpp"
#include "../../../objects/Player.hpp"
#include "../../../util/stringutil.hpp"
#include "../../../voxels/Block.hpp"
#include "../../../voxels/Chunks.hpp"
#include "../../../voxels/voxel.hpp"
#include "../../../world/Level.hpp"

#include <iostream>
#include <glm/glm.hpp>

namespace scripting {
    extern Hud* hud;
}

static int l_hud_open_inventory(lua_State*) {
    if (!scripting::hud->isInventoryOpen()) {
        scripting::hud->openInventory();
    }
    return 0;
}

static int l_hud_close_inventory(lua_State*) {
    if (scripting::hud->isInventoryOpen()) {
        scripting::hud->closeInventory();
    }
    return 0;
}

static int l_hud_open_block(lua_State* L) {
    auto x = lua_tointeger(L, 1);
    auto y = lua_tointeger(L, 2);
    auto z = lua_tointeger(L, 3);
    bool playerInventory = !lua_toboolean(L, 4);

    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        throw std::runtime_error("block does not exists at " +
            std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z)
        );
    }
    auto def = scripting::content->getIndices()->getBlockDef(vox->id);
    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(def->uiLayout);
    if (layout == nullptr) {
        throw std::runtime_error("block '"+def->name+"' has no ui layout");
    }

    auto id = scripting::blocks->createBlockInventory(x, y, z);
    scripting::hud->openInventory(
        glm::ivec3(x, y, z), layout, scripting::level->inventories->get(id), playerInventory
    );

    lua_pushinteger(L, id);
    lua_pushstring(L, def->uiLayout.c_str());
    return 2;
}

static int l_hud_show_overlay(lua_State* L) {
    const char* name = lua_tostring(L, 1);
    bool playerInventory = lua_toboolean(L, 2);

    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(name);
    if (layout == nullptr) {
        throw std::runtime_error("there is no ui layout "+util::quote(name));
    }
    scripting::hud->showOverlay(layout, playerInventory);
    return 0;
}

static UiDocument* require_layout(lua_State* L, const char* name) {
    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(name);
    if (layout == nullptr) {
        throw std::runtime_error("layout '"+std::string(name)+"' is not found");
    }
    return layout;
}

static int l_hud_open_permanent(lua_State* L) {
    auto layout = require_layout(L, lua_tostring(L, 1));
    scripting::hud->openPermanent(layout);
    return 0;
}

static int l_hud_close(lua_State* L) {
    auto layout = require_layout(L, lua_tostring(L, 1));
    scripting::hud->remove(layout->getRoot());
    return 0;
}

static int l_hud_pause(lua_State*) {
    scripting::hud->setPause(true);
    return 0;
}

static int l_hud_resume(lua_State*) {
    scripting::hud->setPause(false);
    return 0;
}

static int l_hud_get_block_inventory(lua_State* L) {
    auto inventory = scripting::hud->getBlockInventory();
    if (inventory == nullptr) {
        lua_pushinteger(L, 0);
    } else {
        lua_pushinteger(L, inventory->getId());
    }
    return 1;
}

static int l_hud_get_player(lua_State* L) {
    auto player = scripting::hud->getPlayer();
    lua_pushinteger(L, player->getId());
    return 1;
}

const luaL_Reg hudlib [] = {
    {"open_inventory", lua::wrap<l_hud_open_inventory>},
    {"close_inventory", lua::wrap<l_hud_close_inventory>},
    {"open_block", lua::wrap<l_hud_open_block>},
    {"open_permanent", lua::wrap<l_hud_open_permanent>},
    {"show_overlay", lua::wrap<l_hud_show_overlay>},
    {"get_block_inventory", lua::wrap<l_hud_get_block_inventory>},
    {"close", lua::wrap<l_hud_close>},
    {"pause", lua::wrap<l_hud_pause>},
    {"resume", lua::wrap<l_hud_resume>},
    {"get_player", lua::wrap<l_hud_get_player>},
    {NULL, NULL}
};
