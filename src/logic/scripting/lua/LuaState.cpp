#include "LuaState.hpp"

#include "lua_util.hpp"
#include "api_lua.hpp"
#include "../../../debug/Logger.hpp"
#include "../../../util/stringutil.hpp"

#include <iomanip>
#include <iostream>

inline std::string LAMBDAS_TABLE = "$L";

static debug::Logger logger("lua-state");

using namespace lua;

namespace scripting {
    extern LuaState* state;
}

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
    setglobal(L, envName(0));

    lua_createtable(L, 0, 0);
    setglobal(L, LAMBDAS_TABLE);
}

std::string LuaState::envName(int env) {
    return "_ENV"+util::mangleid(env);
}

LuaState::~LuaState() {
    lua_close(mainThread);
}

void LuaState::addfunc(lua_State* L, const std::string& name, lua_CFunction func) {
    lua_pushcfunction(L, func);
    lua_setglobal(L, name.c_str());
}

bool LuaState::rename(lua_State* L, const std::string& from, const std::string& to) {
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

void LuaState::remove(lua_State* L, const std::string& name) {
    lua_pushnil(L);
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

    addfunc(L, "print", lua_wrap_errors<l_print>);
}

void LuaState::loadbuffer(lua_State* L, int env, const std::string& src, const std::string& file) {
    if (luaL_loadbuffer(L, src.c_str(), src.length(), file.c_str())) {
        throw luaerror(lua_tostring(L, -1));
    }
    if (env && getglobal(L, envName(env))) {
        lua_setfenv(L, -2);
    }
}

int LuaState::eval(lua_State* L, int env, const std::string& src, const std::string& file) {
    auto srcText = "return "+src;
    loadbuffer(L, env, srcText, file);
    return call(L, 0);
}

int LuaState::execute(lua_State* L, int env, const std::string& src, const std::string& file) {
    loadbuffer(L, env, src, file);
    return callNoThrow(L, 0);
}

int LuaState::pushenv(lua_State* L, int env) {
    if (getglobal(L, envName(env))) {
        return 1;
    }
    return 0;
}

void LuaState::openlib(lua_State* L, const std::string& name, const luaL_Reg* libfuncs) {
    lua_newtable(L);
    luaL_setfuncs(L, libfuncs, 0);
    lua_setglobal(L, name.c_str());
}

std::shared_ptr<std::string> LuaState::createLambdaHandler(lua_State* L) {
    auto ptr = reinterpret_cast<ptrdiff_t>(lua_topointer(L, -1));
    auto name = util::mangleid(ptr);
    lua_getglobal(L, LAMBDAS_TABLE.c_str());
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, name.c_str());
    lua_pop(L, 2);

    return std::shared_ptr<std::string>(new std::string(name), [=](auto* name) {
        lua_getglobal(L, LAMBDAS_TABLE.c_str());
        lua_pushnil(L);
        lua_setfield(L, -2, name->c_str());
        lua_pop(L, 1);
        delete name;
    });
}

runnable LuaState::createRunnable(lua_State* L) {
    auto funcptr = createLambdaHandler(L);
    return [=]() {
        lua_getglobal(L, LAMBDAS_TABLE.c_str());
        lua_getfield(L, -1, funcptr->c_str());
        callNoThrow(L, 0);
    };
}

scripting::common_func LuaState::createLambda(lua_State* L) {
    auto funcptr = createLambdaHandler(L);
    return [=](const std::vector<dynamic::Value>& args) {
        lua_getglobal(L, LAMBDAS_TABLE.c_str());
        lua_getfield(L, -1, funcptr->c_str());
        for (const auto& arg : args) {
            pushvalue(L, arg);
        }
        if (call(L, args.size(), 1)) {
            auto result = tovalue(L, -1);
            lua_pop(L, 1);
            return result;
        }
        return dynamic::Value(dynamic::NONE);
    };
}

int LuaState::createEnvironment(lua_State* L, int parent) {
    int id = nextEnvironment++;

    // local env = {}
    lua_createtable(L, 0, 1);
    
    // setmetatable(env, {__index=_G})
    lua_createtable(L, 0, 1);
    if (parent == 0) {
        lua_pushvalue(L, LUA_GLOBALSINDEX);
    } else {
        if (pushenv(L, parent) == 0) {
            lua_pushvalue(L, LUA_GLOBALSINDEX);
        }
    }
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);

    // envname = env
    setglobal(L, envName(id));
    return id;
}


void LuaState::removeEnvironment(lua_State* L, int id) {
    if (id == 0) {
        return;
    }
    lua_pushnil(L);
    setglobal(L, envName(id));
}

bool LuaState::emitEvent(lua_State* L, const std::string &name, std::function<int(lua_State*)> args) {
    getglobal(L, "events");
    getfield(L, "emit");
    lua::pushstring(L, name);
    callNoThrow(L, args(L) + 1);
    bool result = lua_toboolean(L, -1);
    lua_pop(L, 2);
    return result;
}


void LuaState::dumpStack(lua_State* L) {
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++) {
        std::cout << std::setw(3) << i << std::setw(20) << luaL_typename(L, i) << std::setw(30);
        switch (lua_type(L, i)) {
            case LUA_TNUMBER:
                std::cout << lua_tonumber(L, i);
                break;
            case LUA_TSTRING:
                std::cout << lua_tostring(L, i);
                break;
            case LUA_TBOOLEAN:
                std::cout << (lua_toboolean(L, i) ? "true" : "false");
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

lua_State* LuaState::getMainThread() const {
    return mainThread;
}
