#include "LuaState.h"

#include "api_lua.h"

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
}

lua::LuaState::~LuaState() {
    lua_close(L);
}

void lua::LuaState::logError(const std::string& text) {
    std::cerr << text << std::endl;
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
    auto funcName = "F$"+std::to_string(funcId);
    lua_setglobal(L, funcName.c_str());
    return funcName;
}