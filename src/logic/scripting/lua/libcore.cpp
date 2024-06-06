#include "lua_commons.hpp"
#include "api_lua.hpp"
#include "LuaState.hpp"

#include "../../../engine.hpp"
#include "../../../files/settings_io.hpp"
#include "../../../files/engine_paths.hpp"
#include "../../../frontend/menu.hpp"
#include "../../../frontend/screens/MenuScreen.hpp"
#include "../../../logic/LevelController.hpp"
#include "../../../logic/EngineController.hpp"
#include "../../../world/Level.hpp"
#include "../../../window/Events.hpp"
#include "../../../window/Window.hpp"
#include "../../../world/WorldGenerators.hpp"
#include "../scripting.hpp"

#include <vector>
#include <memory>

namespace scripting {
    extern lua::LuaState* state;
}

static int l_new_world(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto seed = lua_tostring(L, 2);
    auto generator = lua_tostring(L, 3);
    auto controller = scripting::engine->getController();
    controller->createWorld(name, seed, generator);
    return 0;
}

static int l_open_world(lua_State* L) {
    auto name = lua_tostring(L, 1);

    auto controller = scripting::engine->getController();
    controller->openWorld(name, false);
    return 0;
}

static int l_reopen_world(lua_State*) {
    auto controller = scripting::engine->getController();
    controller->reopenWorld(scripting::level->getWorld());
    return 0;
}

static int l_close_world(lua_State* L) {
    if (scripting::controller == nullptr) {
        throw std::runtime_error("no world open");
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
    auto controller = scripting::engine->getController();
    controller->deleteWorld(name);
    return 0;
}

static int l_reconfig_packs(lua_State* L) {
    if (!lua_istable(L, 1)) {
        throw std::runtime_error("strings array expected as the first argument");
    }
    if (!lua_istable(L, 2)) {
        throw std::runtime_error("strings array expected as the second argument");
    }
    std::vector<std::string> addPacks;
    if (!lua_istable(L, 1)) {
        throw std::runtime_error("an array expected as argument 1");
    }
    int addLen = lua_objlen(L, 1);
    for (int i = 0; i < addLen; i++) {
        lua_rawgeti(L, 1, i+1);
        addPacks.push_back(lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    std::vector<std::string> remPacks;
    if (!lua_istable(L, 2)) {
        throw std::runtime_error("an array expected as argument 2");
    }
    int remLen = lua_objlen(L, 2);
    for (int i = 0; i < remLen; i++) {
        lua_rawgeti(L, 2, i+1);
        remPacks.push_back(lua_tostring(L, -1));
        lua_pop(L, 1);
    }
    auto controller = scripting::engine->getController();
    controller->reconfigPacks(scripting::controller, addPacks, remPacks);
    return 0;
}

static int l_get_setting(lua_State* L) {
    auto name = lua_tostring(L, 1);
    const auto value = scripting::engine->getSettingsHandler().getValue(name);
    scripting::state->pushvalue(std::move(value));
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

static int l_get_setting_info(lua_State* L) {
    auto name = lua_tostring(L, 1);
    auto setting = scripting::engine->getSettingsHandler().getSetting(name);
    lua_createtable(L, 0, 1);
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        lua_pushnumber(L, number->getMin());
        lua_setfield(L, -2, "min");
        lua_pushnumber(L, number->getMax());
        lua_setfield(L, -2, "max");
        return 1;
    }
    if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
        lua_pushinteger(L, integer->getMin());
        lua_setfield(L, -2, "min");
        lua_pushinteger(L, integer->getMax());
        lua_setfield(L, -2, "max");
        return 1;
    }
    lua_pop(L, 1);
    throw std::runtime_error("unsupported setting type");
}

static int l_quit(lua_State*) {
    Window::setShouldClose(true);
    return 0;
}

static int l_get_default_generator(lua_State* L) {
    lua_pushstring(L, WorldGenerators::getDefaultGeneratorID().c_str());
    return 1;
}

static int l_get_generators(lua_State* L) {
    const auto& generators = WorldGenerators::getGeneratorsIDs();
    lua_createtable(L, generators.size(), 0);

    int i = 0;
    for (auto& id : generators) {
        lua_pushstring(L, id.c_str());
        lua_rawseti(L, -2, i + 1);
        i++;
    }
    return 1;
}

const luaL_Reg corelib [] = {
    {"new_world", lua_wrap_errors<l_new_world>},
    {"open_world", lua_wrap_errors<l_open_world>},
    {"reopen_world", lua_wrap_errors<l_reopen_world>},
    {"close_world", lua_wrap_errors<l_close_world>},
    {"delete_world", lua_wrap_errors<l_delete_world>},
    {"reconfig_packs", lua_wrap_errors<l_reconfig_packs>},
    {"get_setting", lua_wrap_errors<l_get_setting>},
    {"set_setting", lua_wrap_errors<l_set_setting>},
    {"str_setting", lua_wrap_errors<l_str_setting>},
    {"get_setting_info", lua_wrap_errors<l_get_setting_info>},
    {"quit", lua_wrap_errors<l_quit>},
    {"get_default_generator", lua_wrap_errors<l_get_default_generator>},
    {"get_generators", lua_wrap_errors<l_get_generators>},
    {NULL, NULL}
};
