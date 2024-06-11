#include "LuaState.hpp"

#include "lua_util.hpp"
#include "api_lua.hpp"
#include "../../../debug/Logger.hpp"
#include "../../../util/stringutil.hpp"

#include <iomanip>
#include <iostream>

static debug::Logger logger("lua-state");

using namespace lua;

luaerror::luaerror(const std::string& message) : std::runtime_error(message) {
}

void LuaState::removeLibFuncs(lua_State* L, const char* libname, const char* funcs[]) {
    if (getglobal(L, libname)) {
        for (uint i = 0; funcs[i]; i++) {
            lua_pushnil(L);
            setfield(L, funcs[i], -2);
        }
    }
}

LuaState::LuaState() {
    logger.info() << LUA_VERSION;
    logger.info() << LUAJIT_VERSION;

    auto L = luaL_newstate();
    if (L == nullptr) {
        throw luaerror("could not to initialize Lua");
    }
    mainThread = L;
    // Allowed standard libraries
    luaopen_base(L);
    luaopen_math(L);
    luaopen_string(L);
    luaopen_table(L);
    luaopen_debug(L);
    luaopen_jit(L);
    luaopen_bit(L);

    luaopen_os(L);
    const char* removed_os[] {
        "execute",
        "exit",
        "remove",
        "rename",
        "setlocale",
        "tmpname",
        nullptr
    };
    removeLibFuncs(L, "os", removed_os);

    createLibs(L);

    pushglobals(L);
    setglobal(L, env_name(0));

    lua_createtable(L, 0, 0);
    setglobal(L, LAMBDAS_TABLE);
}

LuaState::~LuaState() {
    lua_close(mainThread);
}

void LuaState::addfunc(lua_State* L, const std::string& name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, name.c_str());
}

void LuaState::createLibs(lua_State* L) {
    openlib(L, "audio", audiolib);
    openlib(L, "block", blocklib);
    openlib(L, "console", consolelib);
    openlib(L, "core", corelib);
    openlib(L, "file", filelib);
    openlib(L, "gui", guilib);
    openlib(L, "input", inputlib);
    openlib(L, "inventory", inventorylib);
    openlib(L, "item", itemlib);
    openlib(L, "json", jsonlib);
    openlib(L, "pack", packlib);
    openlib(L, "player", playerlib);
    openlib(L, "time", timelib);
    openlib(L, "toml", tomllib);
    openlib(L, "world", worldlib);

    addfunc(L, "print", lua::wrap<l_print>);
}

void LuaState::openlib(lua_State* L, const std::string& name, const luaL_Reg* libfuncs) {
    lua_newtable(L);
    luaL_setfuncs(L, libfuncs, 0);
    lua_setglobal(L, name.c_str());
}

bool LuaState::emitEvent(lua_State* L, const std::string &name, std::function<int(lua_State*)> args) {
    getglobal(L, "events");
    getfield(L, "emit");
    pushstring(L, name);
    call_nothrow(L, args(L) + 1);
    bool result = toboolean(L, -1);
    pop(L, 2);
    return result;
}

lua_State* LuaState::getMainThread() const {
    return mainThread;
}
