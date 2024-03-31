#include "LuaState.h"

#include <iomanip>
#include <iostream>
#include "lua_util.h"
#include "api_lua.h"
#include "../../../util/stringutil.h"

lua::luaerror::luaerror(const std::string& message) : std::runtime_error(message) {
}

void lua::LuaState::removeLibFuncs(const char* libname, const char* funcs[]) {
    if (getglobal(libname)) {
        for (uint i = 0; funcs[i]; i++) {
            pushnil();
            setfield(funcs[i], -2);
        }
    }
}

lua::LuaState::LuaState() {
    std::cout << LUA_VERSION << std::endl;
    std::cout << LUAJIT_VERSION << std::endl;

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
    removeLibFuncs("os", removed_os);

    createLibs();

    lua_pushvalue(L, LUA_GLOBALSINDEX);
    setglobal(envName(0));
}

const std::string lua::LuaState::envName(int env) {
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

void lua::LuaState::createLibs() {
    openlib("audio", audiolib, 0);
    openlib("block", blocklib, 0);
    openlib("core", corelib, 0);
    openlib("file", filelib, 0);
    openlib("gui", guilib, 0);
    openlib("inventory", inventorylib, 0);
    openlib("item", itemlib, 0);
    openlib("pack", packlib, 0);
    openlib("player", playerlib, 0);
    openlib("time", timelib, 0);
    openlib("world", worldlib, 0);

    addfunc("print", lua_wrap_errors<l_print>);
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

int lua::LuaState::pushboolean(bool x) {
    lua_pushboolean(L, x);
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

int lua::LuaState::pushvalue(const dynamic::Value& value) {
    using dynamic::valtype;
    switch (value.type) {
        case valtype::boolean:
            pushboolean(std::get<bool>(value.value));
            break;
        case valtype::integer:
            pushinteger(std::get<integer_t>(value.value));
            break;
        case valtype::number:
            pushnumber(std::get<number_t>(value.value));
            break;
        case valtype::string:
            pushstring(std::get<std::string>(value.value).c_str());
            break;
        case valtype::none:
            pushnil();
            break;
        case valtype::list:
            throw std::runtime_error("type 'list' is not implemented");
        case valtype::map:
            throw std::runtime_error("type 'map' is not implemented");
    }
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

bool lua::LuaState::getfield(const std::string& name, int idx) {
    lua_getfield(L, idx, name.c_str());
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

const char* lua::LuaState::tostring(int idx) {
    return lua_tostring(L, idx);
}

dynamic::Value lua::LuaState::tovalue(int idx) {
    using dynamic::valtype;
    auto type = lua_type(L, idx);
    switch (type) {
        case LUA_TNIL:
        case LUA_TNONE:
            return dynamic::Value(valtype::none, 0);
        case LUA_TBOOLEAN:
            return dynamic::Value::of(lua_toboolean(L, idx) == 1);
        case LUA_TNUMBER: {
            auto number = lua_tonumber(L, idx);
            auto integer = lua_tointeger(L, idx);
            if (number == (lua_Number)integer) {
                return dynamic::Value::of(integer);
            } else {
                return dynamic::Value::of(number);
            }
        }
        case LUA_TSTRING:
            return dynamic::Value::of(lua_tostring(L, idx));
        default:
            throw std::runtime_error("lua type "+std::to_string(type)+" is not supported");
    }
}

bool lua::LuaState::isstring(int idx) {
    return lua_isstring(L, idx);
}

bool lua::LuaState::isfunction(int idx) {
    return lua_isfunction(L, idx);
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

void lua::LuaState::dumpStack() {
    int top = gettop();
    for (int i = 1; i <= top; i++) {
        std::cout << std::setw(3) << i << std::setw(20) << luaL_typename(L, i) << std::setw(30);
        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                std::cout << tonumber(i);
                break;
            case LUA_TSTRING:
                std::cout << tostring(i);
                break;
            case LUA_TBOOLEAN:
                std::cout << (toboolean(i) ? "true" : "false");
                break;
            case LUA_TNIL:
                std::cout << "nil";
                break;
            default:
                std::cout << lua_topointer(L, i);
                break;
        }
        std::cout << std::endl;
    }
}
