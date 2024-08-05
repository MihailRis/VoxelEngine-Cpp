#include <memory>
#include <vector>

#include <constants.hpp>
#include <engine.hpp>
#include <files/engine_paths.hpp>
#include <files/settings_io.hpp>
#include "../../../frontend/menu.hpp"
#include "../../../frontend/screens/MenuScreen.hpp"
#include <logic/EngineController.hpp>
#include <logic/LevelController.hpp>
#include "../../../window/Events.hpp"
#include "../../../window/Window.hpp"
#include <world/Level.hpp>
#include <world/WorldGenerators.hpp>
#include "api_lua.hpp"

using namespace scripting;

/// @brief Creating new world
/// @param name Name world
/// @param seed Seed world
/// @param generator Type of generation
static int l_new_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto seed = lua::require_string(L, 2);
    auto generator = lua::require_string(L, 3);
    auto controller = engine->getController();
    controller->createWorld(name, seed, generator);
    return 0;
}

/// @brief Open world
/// @param name Name world
static int l_open_world(lua::State* L) {
    auto name = lua::require_string(L, 1);

    auto controller = engine->getController();
    controller->openWorld(name, false);
    return 0;
}

/// @brief Reopen world
static int l_reopen_world(lua::State*) {
    auto controller = engine->getController();
    controller->reopenWorld(level->getWorld());
    return 0;
}

/// @brief Close world
/// @param flag Save world (bool)
static int l_close_world(lua::State* L) {
    if (controller == nullptr) {
        throw std::runtime_error("no world open");
    }
    bool save_world = lua::toboolean(L, 1);
    if (save_world) {
        controller->saveWorld();
    }
    // destroy LevelScreen and run quit callbacks
    engine->setScreen(nullptr);
    // create and go to menu screen
    engine->setScreen(std::make_shared<MenuScreen>(engine));
    return 0;
}

/// @brief Delete world
/// @param name Name world
static int l_delete_world(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto controller = engine->getController();
    controller->deleteWorld(name);
    return 0;
}

/// @brief Reconfigure packs
/// @param addPacks An array of packs to add
/// @param remPacks An array of packs to remove
static int l_reconfig_packs(lua::State* L) {
    if (!lua::istable(L, 1)) {
        throw std::runtime_error("strings array expected as the first argument"
        );
    }
    if (!lua::istable(L, 2)) {
        throw std::runtime_error("strings array expected as the second argument"
        );
    }
    std::vector<std::string> addPacks;
    if (!lua::istable(L, 1)) {
        throw std::runtime_error("an array expected as argument 1");
    }
    int addLen = lua::objlen(L, 1);
    for (int i = 0; i < addLen; i++) {
        lua::rawgeti(L, i + 1, 1);
        addPacks.emplace_back(lua::tostring(L, -1));
        lua::pop(L);
    }

    std::vector<std::string> remPacks;
    if (!lua::istable(L, 2)) {
        throw std::runtime_error("an array expected as argument 2");
    }
    int remLen = lua::objlen(L, 2);
    for (int i = 0; i < remLen; i++) {
        lua::rawgeti(L, i + 1, 2);
        remPacks.emplace_back(lua::tostring(L, -1));
        lua::pop(L);
    }
    auto engine_controller = engine->getController();
    engine_controller->reconfigPacks(controller, addPacks, remPacks);
    return 0;
}

/// @brief Get a setting value
/// @param name The name of the setting
/// @return The value of the setting
static int l_get_setting(lua::State* L) {
    auto name = lua::require_string(L, 1);
    const auto value = engine->getSettingsHandler().getValue(name);
    return lua::pushvalue(L, value);
}

/// @brief Set a setting value
/// @param name The name of the setting
/// @param value The new value for the setting
static int l_set_setting(lua::State* L) {
    auto name = lua::require_string(L, 1);
    const auto value = lua::tovalue(L, 2);
    engine->getSettingsHandler().setValue(name, value);
    return 0;
}

/// @brief Convert a setting value to a string
/// @param name The name of the setting
/// @return The string representation of the setting value
static int l_str_setting(lua::State* L) {
    auto name = lua::require_string(L, 1);
    const auto string = engine->getSettingsHandler().toString(name);
    return lua::pushstring(L, string);
}

/// @brief Get information about a setting
/// @param name The name of the setting
/// @return A table with information about the setting
static int l_get_setting_info(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto setting = engine->getSettingsHandler().getSetting(name);
    lua::createtable(L, 0, 1);
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        lua::pushnumber(L, number->getMin());
        lua::setfield(L, "min");
        lua::pushnumber(L, number->getMax());
        lua::setfield(L, "max");
        return 1;
    }
    if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
        lua::pushinteger(L, integer->getMin());
        lua::setfield(L, "min");
        lua::pushinteger(L, integer->getMax());
        lua::setfield(L, "max");
        return 1;
    }
    lua::pop(L);
    throw std::runtime_error("unsupported setting type");
}

/// @brief Quit the game
static int l_quit(lua::State*) {
    Window::setShouldClose(true);
    return 0;
}

/// @brief Get the default world generator
/// @return The ID of the default world generator
static int l_get_default_generator(lua::State* L) {
    return lua::pushstring(L, WorldGenerators::getDefaultGeneratorID());
}

/// @brief Get a list of all world generators
/// @return A table with the IDs of all world generators
static int l_get_generators(lua::State* L) {
    const auto& generators = WorldGenerators::getGeneratorsIDs();
    lua::createtable(L, generators.size(), 0);

    int i = 0;
    for (auto& id : generators) {
        lua::pushstring(L, id);
        lua::rawseti(L, i + 1);
        i++;
    }
    return 1;
}

const luaL_Reg corelib[] = {
    {"new_world", lua::wrap<l_new_world>},
    {"open_world", lua::wrap<l_open_world>},
    {"reopen_world", lua::wrap<l_reopen_world>},
    {"close_world", lua::wrap<l_close_world>},
    {"delete_world", lua::wrap<l_delete_world>},
    {"reconfig_packs", lua::wrap<l_reconfig_packs>},
    {"get_setting", lua::wrap<l_get_setting>},
    {"set_setting", lua::wrap<l_set_setting>},
    {"str_setting", lua::wrap<l_str_setting>},
    {"get_setting_info", lua::wrap<l_get_setting_info>},
    {"quit", lua::wrap<l_quit>},
    {"get_default_generator", lua::wrap<l_get_default_generator>},
    {"get_generators", lua::wrap<l_get_generators>},
    {NULL, NULL}};
