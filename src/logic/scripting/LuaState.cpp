#include "LuaState.h"

#include "api_lua.h"
#include "../../util/stringutil.h"

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

    std::cout << LUA_VERSION << std::endl;
#   ifdef LUAJIT_VERSION
        luaopen_jit(L);
        std::cout << LUAJIT_VERSION << std::endl;
#   endif // LUAJIT_VERSION

    createFuncs();

    lua_getglobal(L, "_G");
    lua_setglobal(L, ":G");
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
    openlib("time", timelib, 0);
    openlib("file", filelib, 0);

    addfunc("print", l_print);

    addfunc("block_index", l_block_index);
    addfunc("block_name", l_block_name);
    addfunc("blocks_count", l_blocks_count);
    addfunc("is_solid_at", l_is_solid_at);
    addfunc("is_replaceable_at", l_is_replaceable_at);
    addfunc("set_block", l_set_block);
    addfunc("get_block", l_get_block);
    addfunc("get_block_X", l_get_block_x);
    addfunc("get_block_Y", l_get_block_y);
    addfunc("get_block_Z", l_get_block_z);
    addfunc("get_block_states", l_get_block_states);
    addfunc("get_block_user_bits", l_get_block_user_bits);
    addfunc("set_block_user_bits", l_set_block_user_bits);
}

void lua::LuaState::loadbuffer(const std::string& src, const std::string& file) {
    if (luaL_loadbuffer(L, src.c_str(), src.length(), file.c_str())) {
        throw lua::luaerror(lua_tostring(L, -1));
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

int lua::LuaState::eval(const std::string& src, const std::string& file) {
    auto srcText = "return ("+src+")";
    loadbuffer(srcText, file);
    return call(0);
}

int lua::LuaState::execute(const std::string& src, const std::string& file) {
    loadbuffer(src, file);
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

bool lua::LuaState::toboolean(int index) {
    return lua_toboolean(L, index);
}

lua::luaint lua::LuaState::tointeger(int index) {
    return lua_tointeger(L, index);
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

void lua::LuaState::initEnvironment() {
    lua_getglobal(L, "_G");
    lua_setfield(L, -1, ":G");
}

int lua::LuaState::createEnvironment() {
    int id = nextEnvironment++;

    if (currentEnvironment != 0) {
        setEnvironment(0);
    }

    lua_createtable(L, 0, 0);
    initEnvironment();
    setglobal("_N"+util::mangleid(id));

    setEnvironment(id);
    return id;
}

void lua::LuaState::removeEnvironment(int id) {
    if (currentEnvironment == id) {
        setEnvironment(0);
    }
    lua_pushnil(L);
    setglobal("_N"+util::mangleid(id));
}

int lua::LuaState::getEnvironment() const {
    return currentEnvironment;
}

void lua::LuaState::setEnvironment(int id) {
    if (id == 0) {
        getglobal(":G");
        lua_setfenv(L, -1);
    } else {
        getglobal(":G");
        lua_getfield(L, -1, ("_N"+util::mangleid(id)).c_str());
        if (lua_isnil(L, -1)) {
            lua_pop(L, -1);
            throw luaerror("environment "+std::to_string(id)+" was not found");
        }
        lua_setfenv(L, -1);
    }
}
