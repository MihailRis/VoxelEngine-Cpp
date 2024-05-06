#include "lua_commons.hpp"
#include "api_lua.hpp"
#include "../scripting.hpp"
#include "../../../engine.hpp"
#include "../../../files/files.hpp"
#include "../../../files/engine_paths.hpp"

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

static fs::path resolve_path(lua_State* L, const std::string& path) {
    try {
        return scripting::engine->getPaths()->resolve(path);
    } catch (const files_access_error& err) {
        luaL_error(L, err.what());
        abort(); // unreachable
    }
}

static int l_file_find(lua_State* L) {
    std::string path = lua_tostring(L, 1);
    lua_pushstring(L, scripting::engine->getResPaths()->findRaw(path).c_str());
    return 1;
}

static int l_file_resolve(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushstring(L, path.u8string().c_str());
    return 1;
}

static int l_file_read(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    if (fs::is_regular_file(path)) {
        lua_pushstring(L, files::read_string(path).c_str());
        return 1;
    }
    return luaL_error(L, "file does not exists '%s'", path.u8string().c_str());
}

static int l_file_write(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    const char* text = lua_tostring(L, 2);
    files::write_string(path, text);
    return 1;    
}

static int l_file_exists(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::exists(path));
    return 1;
}

static int l_file_isfile(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::is_regular_file(path));
    return 1;
}

static int l_file_isdir(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::is_directory(path));
    return 1;
}

static int l_file_length(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    if (fs::exists(path)){
        lua_pushinteger(L, fs::file_size(path));
    } else {
        lua_pushinteger(L, -1);
    }
    return 1;
}

static int l_file_mkdir(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::create_directory(path));
    return 1;    
}

static int l_file_mkdirs(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    lua_pushboolean(L, fs::create_directories(path));
    return 1;    
}

static int l_file_read_bytes(lua_State* L) {
    fs::path path = resolve_path(L, lua_tostring(L, 1));
    if (fs::is_regular_file(path)) {
        size_t length = static_cast<size_t>(fs::file_size(path));

        auto bytes = files::read_bytes(path, length);

        lua_createtable(L, length, 0);
        int newTable = lua_gettop(L);

        for(size_t i = 0; i < length; i++) {
            lua_pushnumber(L, bytes[i]);
            lua_rawseti(L, newTable, i+1);
        }
        return 1;
    }
    return luaL_error(L, "file does not exists '%s'", path.u8string().c_str());   
}

static int read_bytes_from_table(lua_State* L, int tableIndex, std::vector<ubyte>& bytes) {
    if(!lua_istable(L, tableIndex)) {
        return luaL_error(L, "table expected");
    } else {
        lua_pushnil(L);

        while(lua_next(L, tableIndex - 1) != 0) {
            const int byte = lua_tointeger(L, -1);

            if(byte < 0 || byte > 255) {
                return luaL_error(L, "invalid byte '%i'", byte);
            }

            bytes.push_back(byte);
                
            lua_pop(L, 1);
        }

        return 1;
    }
}

static int l_file_write_bytes(lua_State* L) {
    int pathIndex = 1;

    if(!lua_isstring(L, pathIndex)) {
        return luaL_error(L, "string expected");
    }

    fs::path path = resolve_path(L, lua_tostring(L, pathIndex));

    std::vector<ubyte> bytes;

    int result = read_bytes_from_table(L, -1, bytes);

    if(result != 1) {
        return result;
    } else {
        lua_pushboolean(L, files::write_bytes(path, bytes.data(), bytes.size()));
        return 1;
    }
}

const luaL_Reg filelib [] = {
    {"resolve", lua_wrap_errors<l_file_resolve>},
    {"find", lua_wrap_errors<l_file_find>},
    {"read", lua_wrap_errors<l_file_read>},
    {"write", lua_wrap_errors<l_file_write>},
    {"exists", lua_wrap_errors<l_file_exists>},
    {"isfile", lua_wrap_errors<l_file_isfile>},
    {"isdir", lua_wrap_errors<l_file_isdir>},
    {"length", lua_wrap_errors<l_file_length>},
    {"mkdir", lua_wrap_errors<l_file_mkdir>},
    {"mkdirs", lua_wrap_errors<l_file_mkdirs>},
    {"read_bytes", lua_wrap_errors<l_file_read_bytes>},
    {"write_bytes", lua_wrap_errors<l_file_write_bytes>},
    {NULL, NULL}
};
