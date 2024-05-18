#include "LuaState.hpp"

#include "lua_util.hpp"
#include "api_lua.hpp"
#include "../../../debug/Logger.hpp"
#include "../../../util/stringutil.hpp"

#include <iomanip>
#include <iostream>

inline std::string LAMBDAS_TABLE = "$L";

static debug::Logger logger("lua-state");

namespace scripting {
    extern lua::LuaState* state;
}

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
    logger.info() << LUA_VERSION;
    logger.info() << LUAJIT_VERSION;

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

    lua_createtable(L, 0, 0);
    setglobal(LAMBDAS_TABLE);
}

const std::string lua::LuaState::envName(int env) {
    return "_ENV"+util::mangleid(env);
}

lua::LuaState::~LuaState() {
    lua_close(L);
}

void lua::LuaState::logError(const std::string& text) {
    logger.error() << text;
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
    openlib("audio", audiolib);
    openlib("block", blocklib);
    openlib("console", consolelib);
    openlib("core", corelib);
    openlib("file", filelib);
    openlib("gui", guilib);
    openlib("input", inputlib);
    openlib("inventory", inventorylib);
    openlib("item", itemlib);
    openlib("json", jsonlib);
    openlib("pack", packlib);
    openlib("player", playerlib);
    openlib("time", timelib);
    openlib("world", worldlib);

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

int lua::LuaState::call(int argc, int nresults) {
    if (lua_pcall(L, argc, nresults, 0)) {
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
    auto srcText = "return "+src;
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
    using namespace dynamic;

    if (auto* flag = std::get_if<bool>(&value)) {
        pushboolean(*flag);
    } else if (auto* num = std::get_if<integer_t>(&value)) {
        pushinteger(*num);
    } else if (auto* num = std::get_if<number_t>(&value)) {
        pushnumber(*num);
    } else if (auto* str = std::get_if<std::string>(&value)) {
        pushstring(str->c_str());
    } else if (auto listptr = std::get_if<List_sptr>(&value)) {
        auto list = *listptr;
        lua_createtable(L, list->size(), 0);
        for (size_t i = 0; i < list->size(); i++) {
            pushvalue(list->get(i));
            lua_rawseti(L, -2, i+1);
        }
    } else if (auto mapptr = std::get_if<Map_sptr>(&value)) {
        auto map = *mapptr;
        lua_createtable(L, 0, map->size());
        for (auto& entry : map->values) {
            pushvalue(entry.second);
            lua_setfield(L, -2, entry.first.c_str());
        }
    } else {
        pushnil();
    }
    return 1;
}

int lua::LuaState::pushglobals() {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    return 1;
}

int lua::LuaState::pushcfunction(lua_CFunction function) {
    lua_pushcfunction(L, function);
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

glm::vec2 lua::LuaState::tovec2(int idx) {
    return lua::tovec2(L, idx);
}

glm::vec4 lua::LuaState::tocolor(int idx) {
    return lua::tocolor(L, idx);
}

const char* lua::LuaState::tostring(int idx) {
    return lua_tostring(L, idx);
}

dynamic::Value lua::LuaState::tovalue(int idx) {
    using namespace dynamic;
    auto type = lua_type(L, idx);
    switch (type) {
        case LUA_TNIL:
        case LUA_TNONE:
            return dynamic::NONE;
        case LUA_TBOOLEAN:
            return lua_toboolean(L, idx) == 1;
        case LUA_TNUMBER: {
            auto number = lua_tonumber(L, idx);
            auto integer = lua_tointeger(L, idx);
            if (number == (lua_Number)integer) {
                return integer;
            } else {
                return number;
            }
        }
        case LUA_TSTRING:
            return std::string(lua_tostring(L, idx));
        case LUA_TTABLE: {
            int len = lua_objlen(L, idx);
            if (len) {
                // array
                auto list = create_list();
                for (int i = 1; i <= len; i++) {
                    lua_rawgeti(L, idx, i);
                    list->put(tovalue(-1));
                    lua_pop(L, 1);
                }
                return list;
            } else {
                // table
                auto map = create_map();
                lua_pushvalue(L, idx);
                lua_pushnil(L);
                while (lua_next(L, -2)) {
                    lua_pushvalue(L, -2);
                    auto key = lua_tostring(L, -1);
                    map->put(key, tovalue(-2));
                    lua_pop(L, 2);
                }
                lua_pop(L, 1);
                return map;
            }
        }
        default:
            throw std::runtime_error(
                "lua type "+std::string(luaL_typename(L, type))+" is not supported"
            );
    }
}

bool lua::LuaState::isstring(int idx) {
    return lua_isstring(L, idx);
}

bool lua::LuaState::isfunction(int idx) {
    return lua_isfunction(L, idx);
}

void lua::LuaState::openlib(const std::string& name, const luaL_Reg* libfuncs) {
    lua_newtable(L);
    luaL_setfuncs(L, libfuncs, 0);
    lua_setglobal(L, name.c_str());
}

std::shared_ptr<std::string> lua::LuaState::createLambdaHandler() {
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

runnable lua::LuaState::createRunnable() {
    auto funcptr = createLambdaHandler();
    return [=]() {
        lua_getglobal(L, LAMBDAS_TABLE.c_str());
        lua_getfield(L, -1, funcptr->c_str());
        callNoThrow(0);
    };
}

scripting::common_func lua::LuaState::createLambda() {
    auto funcptr = createLambdaHandler();
    return [=](const std::vector<dynamic::Value>& args) {
        lua_getglobal(L, LAMBDAS_TABLE.c_str());
        lua_getfield(L, -1, funcptr->c_str());
        for (const auto& arg : args) {
            pushvalue(arg);
        }
        if (call(args.size(), 1)) {
            auto result = tovalue(-1);
            pop(1);
            return result;
        }
        return dynamic::Value(dynamic::NONE);
    };
}

int lua::LuaState::createEnvironment(int parent) {
    int id = nextEnvironment++;

    // local env = {}
    lua_createtable(L, 0, 1);
    
    // setmetatable(env, {__index=_G})
    lua_createtable(L, 0, 1);
    if (parent == 0) {
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
    if (id == 0) {
        return;
    }
    lua_pushnil(L);
    setglobal(envName(id));
}

bool lua::LuaState::emit_event(const std::string &name, std::function<int(lua::LuaState *)> args) {
    getglobal("events");
    getfield("emit");
    pushstring(name);
    callNoThrow(args(this) + 1);
    bool result = toboolean(-1);
    pop(2);
    return result;
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

lua_State* lua::LuaState::getLua() const {
    return L;
}
