#include "api_lua.h"
#include "lua_commons.h"
#include "../scripting.h"
#include "../../../engine.h"
#include "../../../files/engine_paths.h"

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

const luaL_Reg packlib [] = {
    {"get_folder", lua_wrap_errors<l_pack_get_folder>},
    {"get_installed", lua_wrap_errors<l_pack_get_installed>},
    {NULL, NULL}
};
