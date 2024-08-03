#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>

#include "../../../assets/AssetsLoader.hpp"
#include "../../../content/Content.hpp"
#include "../../../engine.hpp"
#include "../../../files/WorldFiles.hpp"
#include "../../../files/engine_paths.hpp"
#include "../../../world/Level.hpp"
#include "../../../world/World.hpp"
#include "api_lua.hpp"

using namespace scripting;

static int l_pack_get_folder(lua::State* L) {
    std::string packName = lua::tostring(L, 1);
    if (packName == "core") {
        auto folder = engine->getPaths()->getResources().u8string() + "/";
        return lua::pushstring(L, folder);
    }
    for (auto& pack : engine->getContentPacks()) {
        if (pack.id == packName) {
            return lua::pushstring(L, pack.folder.u8string() + "/");
        }
    }
    return lua::pushstring(L, "");
}

/// @brief pack.get_installed() -> array<string>
static int l_pack_get_installed(lua::State* L) {
    auto& packs = engine->getContentPacks();
    lua::createtable(L, packs.size(), 0);
    for (size_t i = 0; i < packs.size(); i++) {
        lua::pushstring(L, packs[i].id);
        lua::rawseti(L, i + 1);
    }
    return 1;
}

/// @brief pack.get_available() -> array<string>
static int l_pack_get_available(lua::State* L) {
    fs::path worldFolder("");
    if (level) {
        worldFolder = level->getWorld()->wfile->getFolder();
    }
    auto manager = engine->createPacksManager(worldFolder);
    manager.scan();

    const auto& installed = engine->getContentPacks();
    for (auto& pack : installed) {
        manager.exclude(pack.id);
    }
    auto names = manager.getAllNames();

    lua::createtable(L, names.size(), 0);
    for (size_t i = 0; i < names.size(); i++) {
        lua::pushstring(L, names[i]);
        lua::rawseti(L, i + 1);
    }
    return 1;
}

static int l_pack_get_info(
    lua::State* L, const ContentPack& pack, const Content* content
) {
    lua::createtable(L, 0, 5);

    lua::pushstring(L, pack.id);
    lua::setfield(L, "id");

    lua::pushstring(L, pack.title);
    lua::setfield(L, "title");

    lua::pushstring(L, pack.creator);
    lua::setfield(L, "creator");

    lua::pushstring(L, pack.description);
    lua::setfield(L, "description");

    lua::pushstring(L, pack.version);
    lua::setfield(L, "version");

    auto assets = engine->getAssets();
    std::string icon = pack.id + ".icon";
    if (!AssetsLoader::loadExternalTexture(
            assets, icon, {pack.folder / fs::path("icon.png")}
        )) {
        icon = "gui/no_icon";
    }

    lua::pushstring(L, icon);
    lua::setfield(L, "icon");

    if (!pack.dependencies.empty()) {
        lua::createtable(L, pack.dependencies.size(), 0);
        for (size_t i = 0; i < pack.dependencies.size(); i++) {
            auto& dpack = pack.dependencies[i];
            std::string prefix;
            switch (dpack.level) {
                case DependencyLevel::required:
                    prefix = "!";
                    break;
                case DependencyLevel::optional:
                    prefix = "?";
                    break;
                case DependencyLevel::weak:
                    prefix = "~";
                    break;
                default:
                    throw std::runtime_error("");
            }
            lua::pushfstring(L, "%s%s", prefix.c_str(), dpack.id.c_str());
            lua::rawseti(L, i + 1);
        }
        lua::setfield(L, "dependencies");
    }

    auto runtime = content ? content->getPackRuntime(pack.id) : nullptr;
    if (runtime) {
        lua::pushboolean(L, runtime->getStats().hasSavingContent());
        lua::setfield(L, "has_indices");
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
static int l_pack_get_info(lua::State* L) {
    auto packid = lua::tostring(L, 1);

    auto content = engine->getContent();
    auto& packs = engine->getContentPacks();
    auto found =
        std::find_if(packs.begin(), packs.end(), [packid](const auto& pack) {
            return pack.id == packid;
        });
    if (found == packs.end()) {
        // TODO: optimize
        fs::path worldFolder("");
        if (level) {
            worldFolder = level->getWorld()->wfile->getFolder();
        }
        auto manager = engine->createPacksManager(worldFolder);
        manager.scan();
        auto vec = manager.getAll({packid});
        if (!vec.empty()) {
            return l_pack_get_info(L, vec[0], content);
        }
        return 0;
    }
    const auto& pack = *found;
    return l_pack_get_info(L, pack, content);
}

static int l_pack_get_base_packs(lua::State* L) {
    auto& packs = engine->getBasePacks();
    lua::createtable(L, packs.size(), 0);
    for (size_t i = 0; i < packs.size(); i++) {
        lua::pushstring(L, packs[i]);
        lua::rawseti(L, i + 1);
    }
    return 1;
}

const luaL_Reg packlib[] = {
    {"get_folder", lua::wrap<l_pack_get_folder>},
    {"get_installed", lua::wrap<l_pack_get_installed>},
    {"get_available", lua::wrap<l_pack_get_available>},
    {"get_info", lua::wrap<l_pack_get_info>},
    {"get_base_packs", lua::wrap<l_pack_get_base_packs>},
    {NULL, NULL}};
