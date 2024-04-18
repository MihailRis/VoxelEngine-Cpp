#include "lua_commons.h"
#include "api_lua.h"

#include "../../../engine.h"
#include "../../../files/engine_paths.h"
#include "../../../frontend/menu/menu.h"
#include "../../../frontend/screens.h"
#include "../../../logic/LevelController.h"
#include "../../../window/Events.h"
#include "../../../window/Window.h"
#include "../scripting.h"

#include <vector>
#include <memory>

namespace scripting {
    extern lua::LuaState* state;
}

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

static int l_close_world(lua_State* L) {
    if (scripting::controller == nullptr) {
        luaL_error(L, "no world open");
    }
    bool save_world = lua_toboolean(L, 1);
    if (save_world) {
        scripting::controller->saveWorld();
    }
    // destroy LevelScreen and run quit callbacks
    scripting::engine->setScreen(nullptr);
    // create and go to menu screen
    scripting::engine->setScreen(std::make_shared<MenuScreen>(scripting::engine));
    return 0;
}

static int l_delete_world(lua_State* L) {
    auto name = lua_tostring(L, 1);
    menus::delete_world(name, scripting::engine);
    return 0;
}

static int l_remove_packs(lua_State* L) {
    if (!lua_istable(L, 1)) {
        luaL_error(L, "strings array expected as an argument");
    }
    std::vector<std::string> packs;
    int len = lua_objlen(L, 1);
    for (int i = 0; i < len; i++) {
        lua_rawgeti(L, -1, i+1);
        packs.push_back(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    menus::remove_packs(scripting::engine, scripting::controller, packs);
    return 0;
}

static int l_add_packs(lua_State* L) {
    if (!lua_istable(L, 1)) {
        luaL_error(L, "strings array expected as an argument");
    }
    std::vector<std::string> packs;
    int len = lua_objlen(L, 1);
    for (int i = 0; i < len; i++) {
        lua_rawgeti(L, -1, i+1);
        packs.push_back(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    menus::add_packs(scripting::engine, scripting::controller, packs);
    return 0;
}

static int l_get_bindings(lua_State* L) {
    auto& bindings = Events::bindings;
    lua_createtable(L, bindings.size(), 0);

    int i = 0;
    for (auto& entry : bindings) {
        lua_pushstring(L, entry.first.c_str());
        lua_rawseti(L, -2, i + 1);
        i++;
    }
    return 1;
}

static int l_get_setting(lua_State* L) {
    auto name = lua_tostring(L, 1);
    const auto value = scripting::engine->getSettingsHandler().getValue(name);
    scripting::state->pushvalue(value);
    return 1;
}

static int l_set_setting(lua_State* L) {
    auto name = lua_tostring(L, 1);
    const auto value = scripting::state->tovalue(2);
    scripting::engine->getSettingsHandler().setValue(name, value);
    return 0;
}

static int l_str_setting(lua_State* L) {
    auto name = lua_tostring(L, 1);
    const auto string = scripting::engine->getSettingsHandler().toString(name);
    scripting::state->pushstring(string);
    return 1;
}

static int l_quit(lua_State* L) {
    Window::setShouldClose(true);
    return 0;
}

const luaL_Reg corelib [] = {
    {"get_worlds_list", lua_wrap_errors<l_get_worlds_list>},
    {"open_world", lua_wrap_errors<l_open_world>},
    {"close_world", lua_wrap_errors<l_close_world>},
    {"delete_world", lua_wrap_errors<l_delete_world>},
    {"add_packs", lua_wrap_errors<l_add_packs>},
    {"remove_packs", lua_wrap_errors<l_remove_packs>},
    {"get_bindings", lua_wrap_errors<l_get_bindings>},
    {"get_setting", lua_wrap_errors<l_get_setting>},
    {"set_setting", lua_wrap_errors<l_set_setting>},
    {"str_setting", lua_wrap_errors<l_str_setting>},
    {"quit", lua_wrap_errors<l_quit>},
    {NULL, NULL}
};
