#include "api_lua.hpp"
#include "lua_commons.hpp"

#include "../scripting.hpp"
#include "../../../engine.hpp"
#include "../../../assets/AssetsLoader.hpp"
#include "../../../files/engine_paths.hpp"
#include "../../../files/WorldFiles.hpp"
#include "../../../world/Level.hpp"
#include "../../../world/World.hpp"

#include <string>
#include <stdexcept>
#include <filesystem>

static int l_pack_get_folder(lua_State* L) {
    std::string packName = lua_tostring(L, 1);
    if (packName == "core") {
        auto folder = scripting::engine->getPaths()->getResources().u8string()+"/";
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

/// @brief pack.get_installed() -> array<string>
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
    fs::path worldFolder("");
    if (scripting::level) {
        worldFolder = scripting::level->getWorld()->wfile->getFolder();
    }
    auto manager = scripting::engine->createPacksManager(worldFolder);
    manager.scan();

    const auto& installed = scripting::engine->getContentPacks();
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

    lua_pushstring(L, pack.id.c_str());
    lua_setfield(L, -2, "id");

    lua_pushstring(L, pack.title.c_str());
    lua_setfield(L, -2, "title");

    lua_pushstring(L, pack.creator.c_str());
    lua_setfield(L, -2, "creator");

    lua_pushstring(L, pack.description.c_str());
    lua_setfield(L, -2, "description");

    lua_pushstring(L, pack.version.c_str());
    lua_setfield(L, -2, "version");

    auto assets = scripting::engine->getAssets();
    std::string icon = pack.id+".icon";
    if (!AssetsLoader::loadExternalTexture(assets, icon, {
        pack.folder/fs::path("icon.png")
    })) {
        icon = "gui/no_icon";
    }

    lua_pushstring(L, icon.c_str());
    lua_setfield(L, -2, "icon");

    if (!pack.dependencies.empty()) {
        lua_createtable(L, pack.dependencies.size(), 0);
        for (size_t i = 0; i < pack.dependencies.size(); i++) {
            auto& dpack = pack.dependencies.at(i);
            std::string prefix;
            switch (dpack.level) {
                case DependencyLevel::required: prefix = "!"; break;
                case DependencyLevel::optional: prefix = "?"; break;
                case DependencyLevel::weak: prefix = "~"; break;
                default: throw std::runtime_error("");
            }
            lua_pushfstring(L, "%s%s", prefix.c_str(), dpack.id.c_str());
            lua_rawseti(L, -2, i+1);
        }
        lua_setfield(L, -2, "dependencies");
    }

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
    auto found = std::find_if(packs.begin(), packs.end(), [packid](const auto& pack) {
        return pack.id == packid;
    });
    if (found == packs.end()) {
        // TODO: optimize
        fs::path worldFolder("");
        if (scripting::level) {
            worldFolder = scripting::level->getWorld()->wfile->getFolder();
        }
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

static int l_pack_get_base_packs(lua_State* L) {
    auto& packs = scripting::engine->getBasePacks();
    lua_createtable(L, packs.size(), 0);
    for (size_t i = 0; i < packs.size(); i++) {
        lua_pushstring(L, packs[i].c_str());
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

const luaL_Reg packlib [] = {
    {"get_folder", lua_wrap_errors<l_pack_get_folder>},
    {"get_installed", lua_wrap_errors<l_pack_get_installed>},
    {"get_available", lua_wrap_errors<l_pack_get_available>},
    {"get_info", lua_wrap_errors<l_pack_get_info>},
    {"get_base_packs", lua_wrap_errors<l_pack_get_base_packs>},
    {NULL, NULL}
};
