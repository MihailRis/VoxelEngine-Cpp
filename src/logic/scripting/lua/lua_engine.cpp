#include "lua_engine.hpp"

#include <iomanip>
#include <iostream>

#include "files/files.hpp"
#include "files/engine_paths.hpp"
#include "debug/Logger.hpp"
#include "util/stringutil.hpp"
#include "libs/api_lua.hpp"
#include "lua_custom_types.hpp"
#include "engine.hpp"

static debug::Logger logger("lua-state");
static lua::State* main_thread = nullptr;

using namespace lua;

luaerror::luaerror(const std::string& message) : std::runtime_error(message) {
}

static void remove_lib_funcs(
    State* L, const char* libname, const char* funcs[]
) {
    if (getglobal(L, libname)) {
        for (uint i = 0; funcs[i]; i++) {
            pushnil(L);
            setfield(L, funcs[i], -2);
        }
        pop(L);
    }
}

[[nodiscard]] scriptenv lua::create_environment(State* L) {
    int id = lua::create_environment(L, 0);
    return std::shared_ptr<int>(new int(id), [=](int* id) { //-V508
        lua::remove_environment(L, *id);
        delete id;
    });
}

static void create_libs(State* L, StateType stateType) {
    openlib(L, "base64", base64lib);
    openlib(L, "bjson", bjsonlib);
    openlib(L, "block", blocklib);
    openlib(L, "core", corelib);
    openlib(L, "file", filelib);
    openlib(L, "generation", generationlib);
    openlib(L, "item", itemlib);
    openlib(L, "json", jsonlib);
    openlib(L, "mat4", mat4lib);
    openlib(L, "pack", packlib);
    openlib(L, "quat", quatlib);
    openlib(L, "time", timelib);
    openlib(L, "toml", tomllib);
    openlib(L, "utf8", utf8lib);
    openlib(L, "vec2", vec2lib);
    openlib(L, "vec3", vec3lib);
    openlib(L, "vec4", vec4lib);

    if (stateType == StateType::TEST) {
        openlib(L, "test", testlib);
    }
    if (stateType == StateType::BASE || stateType == StateType::TEST) {
        openlib(L, "gui", guilib);
        openlib(L, "input", inputlib);
        openlib(L, "inventory", inventorylib);
        openlib(L, "world", worldlib);
        openlib(L, "audio", audiolib);
        openlib(L, "console", consolelib);
        openlib(L, "player", playerlib);
        openlib(L, "network", networklib);

        openlib(L, "entities", entitylib);
        openlib(L, "cameras", cameralib);

        // components
        openlib(L, "__skeleton", skeletonlib);
        openlib(L, "__rigidbody", rigidbodylib);
        openlib(L, "__transform", transformlib);
    }

    addfunc(L, "print", lua::wrap<l_print>);
}

void lua::init_state(State* L, StateType stateType) {
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
    create_libs(L, stateType);

    pushglobals(L);
    setglobal(L, env_name(0));

    createtable(L, 0, 0);
    setglobal(L, LAMBDAS_TABLE);

    createtable(L, 0, 0);
    setglobal(L, CHUNKS_TABLE);

    initialize_libs_extends(L);

    newusertype<LuaBytearray>(L);
    newusertype<LuaHeightmap>(L);
    newusertype<LuaVoxelFragment>(L);
}

void lua::initialize(const EnginePaths& paths, const CoreParameters& params) {
    logger.info() << LUA_VERSION;
    logger.info() << LUAJIT_VERSION;

    main_thread = create_state(
        paths, params.headless ? StateType::TEST : StateType::BASE
    );
    lua::pushstring(main_thread, params.scriptFile.stem().u8string());
    lua::setglobal(main_thread, "__VC_TEST_NAME");
}

void lua::finalize() {
    lua::close(main_thread);
}

bool lua::emit_event(
    State* L, const std::string& name, std::function<int(State*)> args
) {
    getglobal(L, "events");
    getfield(L, "emit");
    pushstring(L, name);
    call_nothrow(L, args(L) + 1);
    bool result = toboolean(L, -1);
    pop(L, 2);
    return result;
}

State* lua::get_main_state() {
    return main_thread;
}

State* lua::create_state(const EnginePaths& paths, StateType stateType) {
    auto L = luaL_newstate();
    if (L == nullptr) {
        throw luaerror("could not initialize Lua state");
    }
    init_state(L, stateType);
    
    auto resDir = paths.getResourcesFolder();
    auto file = resDir / fs::u8path("scripts/stdmin.lua");
    auto src = files::read_string(file);
    lua::pop(L, lua::execute(L, 0, src, "core:scripts/stdmin.lua"));
    return L;
}
