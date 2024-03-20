#include "lua_commons.h"
#include "api_lua.h"

#include "../../../engine.h"
#include "../../../files/engine_paths.h"
#include "../../../frontend/menu/menu.h"
#include "../scripting.h"

#include <vector>

static int l_get_worlds_list(lua_State* L) {
    auto paths = scripting::engine->getPaths();
    auto worlds = paths->scanForWorlds();

    lua_createtable(L, worlds.size(), 0);
    for (size_t i = 0; i < worlds.size(); i++) {
        lua_pushstring(L, worlds[i].filename().u8string().c_str());
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static int l_open_world(lua_State* L) {
    auto name = lua_tostring(L, 1);
    scripting::engine->setScreen(nullptr);
    menus::open_world(name, scripting::engine, false);
    return 0;
}

const luaL_Reg corelib [] = {
    {"get_worlds_list", lua_wrap_errors<l_get_worlds_list>},
    {"open_world", lua_wrap_errors<l_open_world>},
    {NULL, NULL}
};
