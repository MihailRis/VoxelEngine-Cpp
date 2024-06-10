#include "lua_commons.hpp"
#include "lua_util.hpp"
#include "api_lua.hpp"
#include "LuaState.hpp"
#include "../scripting.hpp"
#include "../../../engine.hpp"
#include "../../../coders/gzip.hpp"
#include "../../../files/files.hpp"
#include "../../../files/engine_paths.hpp"
#include "../../../util/stringutil.hpp"

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

namespace scripting {
    extern lua::LuaState* state;
}

using namespace scripting;

static fs::path resolve_path(const std::string& path) {
    return engine->getPaths()->resolve(path);
}

static fs::path resolve_path_soft(const std::string& path) {
    if (path.find(':') == std::string::npos) {
        return path;
    }
    return engine->getPaths()->resolve(path, false);
}

static int l_file_find(lua_State* L) {
    auto path = lua::require_string(L, 1);
    try {
        lua_pushstring(L, engine->getResPaths()->findRaw(path).c_str());
        return 1;
    } catch (const std::runtime_error& err) {
        return 0;
    }
}

static int l_file_resolve(lua_State* L) {
    fs::path path = resolve_path(lua::require_string(L, 1));
    lua_pushstring(L, path.u8string().c_str());
    return 1;
}

static int l_file_read(lua_State* L) {
    fs::path path = resolve_path(lua::require_string(L, 1));
    if (fs::is_regular_file(path)) {
        lua_pushstring(L, files::read_string(path).c_str());
        return 1;
    }
    throw std::runtime_error("file does not exists "+util::quote(path.u8string()));
}

static int l_file_write(lua_State* L) {
    fs::path path = resolve_path(lua::require_string(L, 1));
    std::string text = lua::require_string(L, 2);
    files::write_string(path, text);
    return 1;    
}

static int l_file_remove(lua_State* L) {
    std::string rawpath = lua::require_string(L, 1);
    fs::path path = resolve_path(rawpath);
    auto entryPoint = rawpath.substr(0, rawpath.find(':'));
    if (entryPoint != "world") {
        throw std::runtime_error("access denied");
    }
    return lua::pushboolean(L, fs::remove(path));
}

static int l_file_remove_tree(lua_State* L) {
    std::string rawpath = lua::require_string(L, 1);
    fs::path path = resolve_path(rawpath);
    auto entryPoint = rawpath.substr(0, rawpath.find(':'));
    if (entryPoint != "world") {
        throw std::runtime_error("access denied");
    }
    return lua::pushinteger(L, fs::remove_all(path));
}

static int l_file_exists(lua_State* L) {
    fs::path path = resolve_path_soft(lua::require_string(L, 1));
    return lua::pushboolean(L, fs::exists(path));
}

static int l_file_isfile(lua_State* L) {
    fs::path path = resolve_path_soft(lua::require_string(L, 1));
    return lua::pushboolean(L, fs::is_regular_file(path));
}

static int l_file_isdir(lua_State* L) {
    fs::path path = resolve_path_soft(lua::require_string(L, 1));
    return lua::pushboolean(L, fs::is_directory(path));
}

static int l_file_length(lua_State* L) {
    fs::path path = resolve_path(lua::require_string(L, 1));
    if (fs::exists(path)){
        lua_pushinteger(L, fs::file_size(path));
    } else {
        lua_pushinteger(L, -1);
    }
    return 1;
}

static int l_file_mkdir(lua_State* L) {
    fs::path path = resolve_path(lua::require_string(L, 1));
    return lua::pushboolean(L, fs::create_directory(path));  
}

static int l_file_mkdirs(lua_State* L) {
    fs::path path = resolve_path(lua::require_string(L, 1));
    return lua::pushboolean(L, fs::create_directories(path)); 
}

static int l_file_read_bytes(lua_State* L) {
    fs::path path = resolve_path(lua::require_string(L, 1));
    if (fs::is_regular_file(path)) {
        size_t length = static_cast<size_t>(fs::file_size(path));

        auto bytes = files::read_bytes(path, length);

        lua_createtable(L, length, 0);
        int newTable = lua_gettop(L);

        for(size_t i = 0; i < length; i++) {
            lua_pushinteger(L, bytes[i]);
            lua_rawseti(L, newTable, i+1);
        }
        return 1;
    }
    throw std::runtime_error("file does not exists "+util::quote(path.u8string()));   
}

static int read_bytes_from_table(lua_State* L, int tableIndex, std::vector<ubyte>& bytes) {
    if(!lua_istable(L, tableIndex)) {
        throw std::runtime_error("table expected");
    } else {
        lua_pushnil(L);
        while(lua_next(L, tableIndex - 1) != 0) {
            const int byte = lua_tointeger(L, -1);
            if(byte < 0 || byte > 255) {
                throw std::runtime_error("invalid byte '"+std::to_string(byte)+"'");
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
        throw std::runtime_error("string expected");
    }

    fs::path path = resolve_path(lua::require_string(L, pathIndex));

    std::vector<ubyte> bytes;

    int result = read_bytes_from_table(L, -1, bytes);

    if(result != 1) {
        return result;
    } else {
        return lua::pushboolean(L, files::write_bytes(path, bytes.data(), bytes.size()));
    }
}

static int l_file_list_all_res(lua_State* L, const std::string& path) {
    auto files = engine->getResPaths()->listdirRaw(path);
    lua_createtable(L, files.size(), 0);
    for (size_t i = 0; i < files.size(); i++) {
        lua_pushstring(L, files[i].c_str());
        lua_rawseti(L, -2, i+1);
    }
    return 1;
}

static int l_file_list(lua_State* L) {
    std::string dirname = lua::require_string(L, 1);
    if (dirname.find(':') == std::string::npos) {
        return l_file_list_all_res(L, dirname);
    }
    fs::path path = resolve_path(dirname);
    if (!fs::is_directory(path)) {
        throw std::runtime_error(util::quote(path.u8string())+" is not a directory");
    }
    lua_createtable(L, 0, 0);
    size_t index = 1;
    for (auto& entry : fs::directory_iterator(path)) {
        auto name = entry.path().filename().u8string();
        auto file = dirname + "/" + name;
        lua_pushstring(L, file.c_str());
        lua_rawseti(L, -2, index);
        index++;
    }
    return 1;
}

static int l_file_gzip_compress(lua_State* L) { 
    fs::path path = resolve_path(lua::require_string(L, 1)); 
    if (fs::is_regular_file(path)) { 
        size_t length = static_cast<size_t>(fs::file_size(path));

        auto compressed_bytes = gzip::compress(files::read_bytes(path, length).get(), length);

        lua_pushboolean(L, files::write_bytes(path, compressed_bytes.data(), compressed_bytes.size()));
        return 1;
    } 
    throw std::runtime_error("file does not exist " + util::quote(path.u8string())); 
}

static int l_file_gzip_decompress(lua_State* L) { 
    fs::path path = resolve_path(lua::require_string(L, 1)); 
    if (fs::is_regular_file(path)) { 
        size_t length = static_cast<size_t>(fs::file_size(path));

        auto decompressed_bytes = gzip::decompress(files::read_bytes(path, length).get(), length);

        lua_pushboolean(L, files::write_bytes(path, decompressed_bytes.data(), decompressed_bytes.size()));
        return 1;
    } 
    throw std::runtime_error("file does not exist " + util::quote(path.u8string())); 
}

const luaL_Reg filelib [] = {
    {"exists", lua_wrap_errors<l_file_exists>},
    {"find", lua_wrap_errors<l_file_find>},
    {"isdir", lua_wrap_errors<l_file_isdir>},
    {"isfile", lua_wrap_errors<l_file_isfile>},
    {"length", lua_wrap_errors<l_file_length>},
    {"list", lua_wrap_errors<l_file_list>},
    {"mkdir", lua_wrap_errors<l_file_mkdir>},
    {"mkdirs", lua_wrap_errors<l_file_mkdirs>},
    {"read_bytes", lua_wrap_errors<l_file_read_bytes>},
    {"read", lua_wrap_errors<l_file_read>},
    {"remove", lua_wrap_errors<l_file_remove>},
    {"remove_tree", lua_wrap_errors<l_file_remove_tree>},
    {"resolve", lua_wrap_errors<l_file_resolve>},
    {"write_bytes", lua_wrap_errors<l_file_write_bytes>},
    {"write", lua_wrap_errors<l_file_write>},
    {"gzip_compress", lua_wrap_errors<l_file_gzip_compress>},
    {"gzip_decompress", lua_wrap_errors<l_file_gzip_decompress>},
    {NULL, NULL}
};
