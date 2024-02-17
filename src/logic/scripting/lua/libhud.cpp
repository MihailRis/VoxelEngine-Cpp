#include "libhud.h"
#include "LuaState.h"

#include <iostream>
#include <glm/glm.hpp>

#include "../scripting.h"
#include "../../../assets/Assets.h"
#include "../../../frontend/hud.h"
#include "../../../world/Level.h"
#include "../../../voxels/Chunks.h"
#include "../../../voxels/voxel.h"
#include "../../../voxels/Block.h"
#include "../../../content/Content.h"
#include "../../../logic/BlocksController.h"
#include "../../../items/Inventories.h"
#include "../../../engine.h"
#include "../../../frontend/UiDocument.h"

namespace scripting {
    extern Hud* hud;
}

int l_hud_open_inventory(lua_State* L) {
    scripting::hud->openInventory();
    return 0;
}

int l_hud_close_inventory(lua_State* L) {
    scripting::hud->closeInventory();
    return 0;
}

int l_hud_open_block(lua_State* L) {
    lua::luaint x = lua_tointeger(L, 1);
    lua::luaint y = lua_tointeger(L, 2);
    lua::luaint z = lua_tointeger(L, 3);

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
    if (id == 0) {
        luaL_error(L, 
            "block '%s' at %d %d %d has no inventory",
            def->name.c_str(),
            x, y, z
        );
    }
    
    scripting::hud->openInventory(
        glm::ivec3(x, y, z), layout, scripting::level->inventories->get(id)
    );

    lua_pushinteger(L, id);
    lua_pushstring(L, def->uiLayout.c_str());
    return 2;
}

UiDocument* require_layout(lua_State* L, const char* name) {
    auto assets = scripting::engine->getAssets();
    auto layout = assets->getLayout(name);
    if (layout == nullptr) {
        luaL_error(L, "layout '%s' is not found", name);
    }
    return layout;
}

int l_hud_open_permanent(lua_State* L) {
    auto layout = require_layout(L, lua_tostring(L, 1));
    scripting::hud->openPermanent(layout);
    return 0;
}

int l_hud_close(lua_State* L) {
    auto layout = require_layout(L, lua_tostring(L, 1));
    scripting::hud->remove(layout->getRoot());
    return 0;
}
