#include "api_lua.h"
#include "lua_commons.h"
#include "../scripting.h"
#include "../../../engine.h"
#include "../../../files/engine_paths.h"
#include "../../../files/WorldFiles.h"
#include "../../../world/Level.h"
#include "../../../world/World.h"

#include <string>
#include <filesystem>

static int l_pack_get_folder(lua_State* L) {
    std::string packName = lua_tostring(L, 1);
    if (packName == "core") {
        auto folder = scripting::engine->getPaths()
                                       ->getResources().u8string()+"/";
        lua_pushstring(L, folder.c_str());
        return 1;
    }
    for (auto& pack : scripting::engine->getContentPacks()) {
        if (pack.id == packName) {
            lua_pushstring(L, (pack.folder.u8string()+"/").c_str());
            return 1;
        }
    }
    lua_pushstring(L, "");
    return 1;
}

static int l_pack_get_installed(lua_State* L) {
    auto& packs = scripting::engine->getContentPacks();
    lua_createtable(L, packs.size(), 0);
    for (size_t i = 0; i < packs.size(); i++) {
        lua_pushstring(L, packs[i].id.c_str());
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

/// @brief pack.get_available() -> array<string> 
static int l_pack_get_available(lua_State* L) {
    auto worldFolder = scripting::level->getWorld()->wfile->getFolder();
    auto manager = scripting::engine->createPacksManager(worldFolder);
    manager.scan();

    auto& installed = scripting::engine->getContentPacks();
    for (auto& pack : installed) {
        manager.exclude(pack.id);
    }
    auto names = manager.getAllNames();
    
    lua_createtable(L, names.size(), 0);
    for (size_t i = 0; i < names.size(); i++) {
        lua_pushstring(L, names[i].c_str());
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

static int l_pack_get_info(lua_State* L, const ContentPack& pack, const Content* content) {
    lua_createtable(L, 0, 5);

    lua_pushstring(L, pack.title.c_str());
    lua_setfield(L, -2, "title");

    lua_pushstring(L, pack.creator.c_str());
    lua_setfield(L, -2, "creator");

    lua_pushstring(L, pack.description.c_str());
    lua_setfield(L, -2, "description");

    lua_pushstring(L, pack.version.c_str());
    lua_setfield(L, -2, "version");

    auto runtime = content ? content->getPackRuntime(pack.id) : nullptr;
    if (runtime) {
        lua_pushboolean(L, runtime->getStats().hasSavingContent());
        lua_setfield(L, -2, "has_indices");
    }
    return 1;
}

/// @brief pack.get_info(packid: str) -> {
///     title: str,
///     creator: str,
///     description: str,
///     version: str,
///     [optional] has_indices: bool
/// } or nil
static int l_pack_get_info(lua_State* L) {
    auto packid = lua_tostring(L, 1);
    
    auto content = scripting::engine->getContent();
    auto& packs = scripting::engine->getContentPacks();
    auto found = std::find_if(packs.begin(), packs.end(), [packid](auto& pack) {
        return pack.id == packid;
    });
    if (found == packs.end()) {
        // TODO: optimize
        auto worldFolder = scripting::level->getWorld()->wfile->getFolder();
        auto manager = scripting::engine->createPacksManager(worldFolder);
        manager.scan();
        auto vec = manager.getAll({packid});
        if (!vec.empty()) {
            return l_pack_get_info(L, vec.at(0), content);
        }
        return 0;
    }
    const auto& pack = *found;
    return l_pack_get_info(L, pack, content);
}

const luaL_Reg packlib [] = {
    {"get_folder", lua_wrap_errors<l_pack_get_folder>},
    {"get_installed", lua_wrap_errors<l_pack_get_installed>},
    {"get_available", lua_wrap_errors<l_pack_get_available>},
    {"get_info", lua_wrap_errors<l_pack_get_info>},
    {NULL, NULL}
};
