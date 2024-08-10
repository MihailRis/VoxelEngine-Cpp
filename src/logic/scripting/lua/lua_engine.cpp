#include "lua_engine.hpp"

#include <iomanip>
#include <iostream>

#include "debug/Logger.hpp"
#include "util/stringutil.hpp"
#include "api_lua.hpp"
#include "lua_custom_types.hpp"

static debug::Logger logger("lua-state");
static lua::State* main_thread = nullptr;

using namespace lua;

luaerror::luaerror(const std::string& message) : std::runtime_error(message) {
}

static void remove_lib_funcs(
    lua::State* L, const char* libname, const char* funcs[]
) {
    if (getglobal(L, libname)) {
        for (uint i = 0; funcs[i]; i++) {
            pushnil(L);
            setfield(L, funcs[i], -2);
        }
        pop(L);
    }
}

static void create_libs(lua::State* L) {
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
    openlib(L, "mat4", mat4lib);
    openlib(L, "pack", packlib);
    openlib(L, "player", playerlib);
    openlib(L, "quat", quatlib);
    openlib(L, "time", timelib);
    openlib(L, "toml", tomllib);
    openlib(L, "vec2", vec2lib);
    openlib(L, "vec3", vec3lib);
    openlib(L, "vec4", vec4lib);
    openlib(L, "world", worldlib);

    openlib(L, "entities", entitylib);
    openlib(L, "cameras", cameralib);

    // components
    openlib(L, "__skeleton", skeletonlib);
    openlib(L, "__rigidbody", rigidbodylib);
    openlib(L, "__transform", transformlib);

    addfunc(L, "print", lua::wrap<l_print>);
}

void lua::initialize() {
    logger.info() << LUA_VERSION;
    logger.info() << LUAJIT_VERSION;

    auto L = luaL_newstate();
    if (L == nullptr) {
        throw luaerror("could not to initialize Lua");
    }
    main_thread = L;
    // Allowed standard libraries
    pop(L, luaopen_base(L));
    pop(L, luaopen_math(L));
    pop(L, luaopen_string(L));
    pop(L, luaopen_table(L));
    pop(L, luaopen_debug(L));
    pop(L, luaopen_jit(L));
    pop(L, luaopen_bit(L));
    pop(L, luaopen_os(L));
    const char* removed_os[] {
        "execute", "exit", "remove", "rename", "setlocale", "tmpname", nullptr};
    remove_lib_funcs(L, "os", removed_os);
    create_libs(L);

    pushglobals(L);
    setglobal(L, env_name(0));

    createtable(L, 0, 0);
    setglobal(L, LAMBDAS_TABLE);

    createtable(L, 0, 0);
    setglobal(L, CHUNKS_TABLE);

    initialize_libs_extends(L);

    newusertype<Bytearray, Bytearray::createMetatable>(L, "bytearray");
}

void lua::finalize() {
    lua_close(main_thread);
}

bool lua::emit_event(
    lua::State* L, const std::string& name, std::function<int(lua::State*)> args
) {
    getglobal(L, "events");
    getfield(L, "emit");
    pushstring(L, name);
    call_nothrow(L, args(L) + 1);
    bool result = toboolean(L, -1);
    pop(L, 2);
    return result;
}

lua::State* lua::get_main_thread() {
    return main_thread;
}
