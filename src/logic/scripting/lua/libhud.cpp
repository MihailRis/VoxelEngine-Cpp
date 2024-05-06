#include "lua_commons.hpp"
#include "api_lua.hpp"
#include "LuaState.hpp"

#include "../../../assets/Assets.hpp"
#include "../../../content/Content.h"
#include "../../../engine.h"
#include "../../../frontend/hud.hpp"
#include "../../../frontend/UiDocument.hpp"
#include "../../../graphics/ui/elements/InventoryView.hpp"
#include "../../../items/Inventories.h"
#include "../../../logic/BlocksController.h"
#include "../../../voxels/Block.h"
#include "../../../voxels/Chunks.h"
#include "../../../voxels/voxel.h"
#include "../../../world/Level.h"
#include "../scripting.h"

#include <iostream>
#include <glm/glm.hpp>

namespace scripting {
    extern Hud* hud;
}

static int l_hud_open_inventory(lua_State* L) {
    if (!scripting::hud->isInventoryOpen()) {
        scripting::hud->openInventory();
    }
    return 0;
}

static int l_hud_close_inventory(lua_State* L) {
    if (scripting::hud->isInventoryOpen()) {
        scripting::hud->closeInventory();
    }
    return 0;
}

static int l_hud_open_block(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);
    bool playerInventory = !lua_toboolean(L, 4);

    voxel* vox = scripting::level->chunks->get(x, y, z);
    if (vox == nullptr) {
        luaL_error(L, "block does not exists at %d %d %d", x, y, z);
    }
    auto def = scripting::content->getIndices()->getBlockDef(vox->id);
    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(def->uiLayout);
    if (layout == nullptr) {
        luaL_error(L, "block '%s' has no ui layout", def->name.c_str());
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
        luaL_error(L, "there is no ui layout '%s'", name);
    }
    scripting::hud->showOverlay(layout, playerInventory);
    return 0;
}

static UiDocument* require_layout(lua_State* L, const char* name) {
    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(name);
    if (layout == nullptr) {
        luaL_error(L, "layout '%s' is not found", name);
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

static int l_hud_pause(lua_State* L) {
    scripting::hud->setPause(true);
    return 0;
}

static int l_hud_resume(lua_State* L) {
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

const luaL_Reg hudlib [] = {
    {"open_inventory", lua_wrap_errors<l_hud_open_inventory>},
    {"close_inventory", lua_wrap_errors<l_hud_close_inventory>},
    {"open_block", lua_wrap_errors<l_hud_open_block>},
    {"open_permanent", lua_wrap_errors<l_hud_open_permanent>},
    {"show_overlay", lua_wrap_errors<l_hud_show_overlay>},
    {"get_block_inventory", lua_wrap_errors<l_hud_get_block_inventory>},
    {"close", lua_wrap_errors<l_hud_close>},
    {"pause", lua_wrap_errors<l_hud_pause>},
    {"resume", lua_wrap_errors<l_hud_resume>},
    {NULL, NULL}
};
