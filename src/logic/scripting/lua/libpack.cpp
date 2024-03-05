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

const luaL_Reg packlib [] = {
    {"get_folder", lua_wrap_errors<l_pack_get_folder>},
    {NULL, NULL}
};
