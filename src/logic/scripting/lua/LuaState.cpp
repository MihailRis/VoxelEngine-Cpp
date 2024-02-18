#include "LuaState.h"

#include <iostream>
#include "lua_util.h"
#include "api_lua.h"
#include "libgui.h"
#include "../../../util/stringutil.h"

lua::luaerror::luaerror(const std::string& message) : std::runtime_error(message) {
}

lua::LuaState::LuaState() {
    L = luaL_newstate();
    if (L == nullptr) {
        throw lua::luaerror("could not to initialize Lua");
    }
    // Allowed standard libraries
    luaopen_base(L);
    luaopen_math(L);
    luaopen_string(L);
    luaopen_table(L);
    luaopen_debug(L);
    luaopen_jit(L);
    luaopen_bit(L);

    std::cout << LUA_VERSION << std::endl;
    std::cout << LUAJIT_VERSION << std::endl;

    createFuncs();

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    setglobal(envName(0));
}

const std::string lua::LuaState::envName(int env) const {
    return "_ENV"+util::mangleid(env);
}

lua::LuaState::~LuaState() {
    lua_close(L);
}

void lua::LuaState::logError(const std::string& text) {
    std::cerr << text << std::endl;
}

void lua::LuaState::addfunc(const std::string& name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, name.c_str());
}

bool lua::LuaState::getglobal(const std::string& name) {
    lua_getglobal(L, name.c_str());
    if (lua_isnil(L, lua_gettop(L))) {
        lua_pop(L, lua_gettop(L));
        return false;
    }
    return true;
}

bool lua::LuaState::hasglobal(const std::string& name) {
    lua_getglobal(L, name.c_str());
    if (lua_isnil(L, lua_gettop(L))) {
        lua_pop(L, lua_gettop(L));
        return false;
    }
    lua_pop(L, lua_gettop(L));
    return true;
}

void lua::LuaState::setglobal(const std::string& name) {
    lua_setglobal(L, name.c_str());
}

bool lua::LuaState::rename(const std::string& from, const std::string& to) {
    const char* src = from.c_str();
    lua_getglobal(L, src);
    if (lua_isnil(L, lua_gettop(L))) {
        lua_pop(L, lua_gettop(L));
        return false;
    }
    lua_setglobal(L, to.c_str());
    
    // remove previous
    lua_pushnil(L);
    lua_setglobal(L, src);
    return true;
}

void lua::LuaState::remove(const std::string& name) {
    lua_pushnil(L);
    lua_setglobal(L, name.c_str());
}

void lua::LuaState::createFuncs() {
    openlib("pack", packlib, 0);
    openlib("world", worldlib, 0);
    openlib("player", playerlib, 0);
    openlib("inventory", inventorylib, 0);
    openlib("item", itemlib, 0);
    openlib("time", timelib, 0);
    openlib("file", filelib, 0);
    openlib("gui", guilib, 0);

    addfunc("print", lua_wrap_errors<l_print>);

    addfunc("block_index", lua_wrap_errors<l_block_index>);
    addfunc("block_name", lua_wrap_errors<l_block_name>);
    addfunc("blocks_count", lua_wrap_errors<l_blocks_count>);
    addfunc("is_solid_at", lua_wrap_errors<l_is_solid_at>);
    addfunc("is_replaceable_at", lua_wrap_errors<l_is_replaceable_at>);
    addfunc("set_block", lua_wrap_errors<l_set_block>);
    addfunc("get_block", lua_wrap_errors<l_get_block>);
    addfunc("get_block_X", lua_wrap_errors<l_get_block_x>);
    addfunc("get_block_Y", lua_wrap_errors<l_get_block_y>);
    addfunc("get_block_Z", lua_wrap_errors<l_get_block_z>);
    addfunc("get_block_states", lua_wrap_errors<l_get_block_states>);
    addfunc("get_block_user_bits", lua_wrap_errors<l_get_block_user_bits>);
    addfunc("set_block_user_bits", lua_wrap_errors<l_set_block_user_bits>);
}

void lua::LuaState::loadbuffer(int env, const std::string& src, const std::string& file) {
    if (luaL_loadbuffer(L, src.c_str(), src.length(), file.c_str())) {
        throw lua::luaerror(lua_tostring(L, -1));
    }
    if (env && getglobal(envName(env))) {
        lua_setfenv(L, -2);
    }
}

int lua::LuaState::call(int argc) {
    if (lua_pcall(L, argc, LUA_MULTRET, 0)) {
        throw lua::luaerror(lua_tostring(L, -1));
    }
    return 1;
}

int lua::LuaState::callNoThrow(int argc) {
    if (lua_pcall(L, argc, LUA_MULTRET, 0)) {
        logError(lua_tostring(L, -1));
        return 0;
    }
    return 1;
}

int lua::LuaState::eval(int env, const std::string& src, const std::string& file) {
    auto srcText = "return ("+src+")";
    loadbuffer(env, srcText, file);
    return call(0);
}

int lua::LuaState::execute(int env, const std::string& src, const std::string& file) {
    loadbuffer(env, src, file);
    return callNoThrow(0);
}

int lua::LuaState::gettop() const {
    return lua_gettop(L);
}

int lua::LuaState::pushinteger(luaint x) {
    lua_pushinteger(L, x);
    return 1;
}

int lua::LuaState::pushnumber(luanumber x) {
    lua_pushnumber(L, x);
    return 1;
}

int lua::LuaState::pushivec3(luaint x, luaint y, luaint z) {
    lua::pushivec3(L, x, y, z);
    return 3;
}

int lua::LuaState::pushstring(const std::string& str) {
    lua_pushstring(L, str.c_str());
    return 1;
}

int lua::LuaState::pushenv(int env) {
    if (getglobal(envName(env))) {
        return 1;
    }
    return 0;
}

int lua::LuaState::pushvalue(int idx) {
    lua_pushvalue(L, idx);
    return 1;
}

int lua::LuaState::pushglobals() {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    return 1;
}

void lua::LuaState::pop(int n) {
    lua_pop(L, n);
}

int lua::LuaState::pushnil() {
    lua_pushnil(L);
    return 1;
}

bool lua::LuaState::getfield(const std::string& name) {
    lua_getfield(L, -1, name.c_str());
    if (lua_isnil(L, -1)) {
        lua_pop(L, -1);
        return false;
    }
    return true;
}

void lua::LuaState::setfield(const std::string& name, int idx) {
    lua_setfield(L, idx, name.c_str());
}

bool lua::LuaState::toboolean(int idx) {
    return lua_toboolean(L, idx);
}

lua::luaint lua::LuaState::tointeger(int idx) {
    return lua_tointeger(L, idx);
}

lua::luanumber lua::LuaState::tonumber(int idx) {
    return lua_tonumber(L, idx);
}

void lua::LuaState::openlib(const std::string& name, const luaL_Reg* libfuncs, int nup) {
    lua_newtable(L);
    luaL_setfuncs(L, libfuncs, nup);
    lua_setglobal(L, name.c_str());
}

const std::string lua::LuaState::storeAnonymous() {
    auto funcId = uintptr_t(lua_topointer(L, lua_gettop(L)));
    auto funcName = "F$"+util::mangleid(funcId);
    lua_setglobal(L, funcName.c_str());
    return funcName;
}

int lua::LuaState::createEnvironment(int parent) {
    int id = nextEnvironment++;

    // local env = {}
    lua_createtable(L, 0, 1);
    
    // setmetatable(env, {__index=_G})
    lua_createtable(L, 0, 1);
    if (parent == 0 || true) {
        lua_pushvalue(L, LUA_GLOBALSINDEX);
    } else {
        if (pushenv(parent) == 0) {
            lua_pushvalue(L, LUA_GLOBALSINDEX);
        }
    }
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);

    // envname = env
    setglobal(envName(id));
    return id;
}


void lua::LuaState::removeEnvironment(int id) {
    lua_pushnil(L);
    setglobal(envName(id));
}
