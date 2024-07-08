#include "api_lua.hpp"

#include "../../../assets/Assets.hpp"
#include "../../../assets/AssetsLoader.hpp"
#include "../../../files/engine_paths.hpp"
#include "../../../world/Level.hpp"
#include "../../../world/World.hpp"
#include "../../../engine.hpp"

#include <cmath>
#include <filesystem>

using namespace scripting;
namespace fs = std::filesystem;

static int l_world_get_list(lua::State* L) {
    auto paths = engine->getPaths();
    auto worlds = paths->scanForWorlds();

    lua::createtable(L, worlds.size(), 0);
    for (size_t i = 0; i < worlds.size(); i++) {
        lua::createtable(L, 0, 1);
        
        auto name = worlds[i].filename().u8string();
        lua::pushstring(L, name);
        lua::setfield(L, "name");
        
        auto assets = engine->getAssets();
        std::string icon = "world#"+name+".icon";
        if (!AssetsLoader::loadExternalTexture(assets, icon, {
            worlds[i]/fs::path("icon.png"),
            worlds[i]/fs::path("preview.png")
        })) {
            icon = "gui/no_world_icon";
        }
        lua::pushstring(L, icon);
        lua::setfield(L, "icon");
        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_world_get_total_time(lua::State* L) {
    return lua::pushnumber(L, level->getWorld()->totalTime);
}

static int l_world_get_day_time(lua::State* L) {
    return lua::pushnumber(L, level->getWorld()->daytime);
}

static int l_world_set_day_time(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    level->getWorld()->daytime = fmod(value, 1.0);
    return 0;
}

static int l_wolrd_set_vel_time(lua::State* L) {
    auto value = lua::tonumber(L, 1);
    level->getWorld()->daytimeSpeed = std::abs(value);
    return 0;
}

static int l_wolrd_get_vel_time(lua::State* L) {
    return lua::pushnumber(L, level->getWorld()->daytimeSpeed);
}

static int l_world_get_seed(lua::State* L) {
    return lua::pushinteger(L, level->getWorld()->getSeed());
}

static int l_world_exists(lua::State* L) {
    auto name = lua::require_string(L, 1);
    auto worldsDir = engine->getPaths()->getWorldFolder(name);
    return lua::pushboolean(L, fs::is_directory(worldsDir));
}

static int l_world_is_day(lua::State* L) {
    auto daytime = level->getWorld()->daytime;
    return lua::pushboolean(L, daytime >= 0.2 && daytime <= 0.8);
}

static int l_world_is_night(lua::State* L) {
    auto daytime = level->getWorld()->daytime;
    return lua::pushboolean(L, daytime < 0.2 || daytime > 0.8);
}

const luaL_Reg worldlib [] = {
    {"get_list", lua::wrap<l_world_get_list>},
    {"get_total_time", lua::wrap<l_world_get_total_time>},
    {"get_day_time", lua::wrap<l_world_get_day_time>},
    {"set_day_time", lua::wrap<l_world_set_day_time>},
    {"set_vel_time", lua::wrap<l_wolrd_set_vel_time>},
    {"get_vel_time", lua::wrap<l_wolrd_get_vel_time>},
    {"get_seed", lua::wrap<l_world_get_seed>},
    {"is_day", lua::wrap<l_world_is_day>},
    {"is_night", lua::wrap<l_world_is_night>},
    {"exists", lua::wrap<l_world_exists>},
    {NULL, NULL}
};
