#include "scripting.h"

#include <iostream>
#include <stdexcept>
#include <lua.hpp>

#include "../../files/files.h"
#include "../../util/timeutil.h"
#include "../../world/Level.h"
#include "../../voxels/Block.h"

#include "api_lua.h"

using namespace scripting;

namespace scripting {
    extern lua_State* L;
}

lua_State* scripting::L = nullptr;
Level* scripting::level = nullptr;
const Content* scripting::content = nullptr;

void delete_global(lua_State* L, const char* name) {
    lua_pushnil(L);
    lua_setglobal(L, name);
}

bool rename_global(lua_State* L, const char* src, const char* dst) {
    lua_getglobal(L, src);
    if (lua_isnil(L, lua_gettop(L))) {
        lua_pop(L, lua_gettop(L));
        return false;
    }
    lua_setglobal(L, dst);
    delete_global(L, src);
    return true;
}

void call_func(lua_State* L, int argc, const std::string& name) {
    if (lua_pcall(L, argc, LUA_MULTRET, 0) != LUA_OK) {
        std::cerr << "Lua error in " << name << ": ";
        std::cerr << lua_tostring(L,-1) << std::endl;
    }
}

void scripting::initialize() {
    L = luaL_newstate();
    if (L == nullptr) {
        throw std::runtime_error("could not to initialize Lua");
    }
    luaopen_base(L);
    luaopen_math(L);
    luaopen_jit(L);

    std::cout << LUA_VERSION << std::endl;
#   ifdef LUAJIT_VERSION
        std::cout << LUAJIT_VERSION << std::endl;
#   endif // LUAJIT_VERSION

    apilua::create_funcs(L);
}

void scripting::on_world_load(Level* level) {
    scripting::level = level;
    scripting::content = level->content;

    std::string src = files::read_string(fs::path("res/scripts/world.lua"));
    luaL_loadbuffer(L, src.c_str(), src.length(), "res/scripts/world.lua");
    call_func(L, 0, "<script>");
}

void scripting::on_world_quit() {
    scripting::level = nullptr;
    scripting::content = nullptr;
}

void scripting::update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name+".update";
    lua_getglobal(L, name.c_str());
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, z);
    call_func(L, 3, name);
}

void scripting::random_update_block(const Block* block, int x, int y, int z) {
    std::string name = block->name+".randupdate";
    lua_getglobal(L, name.c_str());
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, z);
    call_func(L, 3, name);
}

void scripting::on_block_placed(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name+".placed";
    lua_getglobal(L, name.c_str());
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, z);
    call_func(L, 3, name);
}

void scripting::on_block_broken(Player* player, const Block* block, int x, int y, int z) {
    std::string name = block->name+".broken";
    lua_getglobal(L, name.c_str());
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, z);
    call_func(L, 3, name);
}

void scripting::load_block_script(std::string prefix, fs::path file, block_funcs_set* funcsset) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;
    if (luaL_loadbuffer(L, src.c_str(), src.size(), file.string().c_str()) != LUA_OK) {
        std::cerr << "Lua error:" << lua_tostring(L,-1) << std::endl;
        return;
    }
    call_func(L, 0, "<script>");
    funcsset->init=rename_global(L, "init", (prefix+".init").c_str());
    funcsset->update=rename_global(L, "on_update", (prefix+".update").c_str());
    funcsset->randupdate=rename_global(L, "on_random_update", (prefix+".randupdate").c_str());
    funcsset->onbroken=rename_global(L, "on_broken", (prefix+".broken").c_str());
    funcsset->onplaced=rename_global(L, "on_placed", (prefix+".placed").c_str());
}

void scripting::close() {
    lua_close(L);

    L = nullptr;
    content = nullptr;
    level = nullptr;
}
