#include "scripting_hud.hpp"

#include "debug/Logger.hpp"
#include "engine.hpp"
#include "files/files.hpp"
#include "frontend/hud.hpp"
#include "graphics/render/WorldRenderer.hpp"
#include "objects/Player.hpp"
#include "lua/libs/api_lua.hpp"
#include "lua/lua_engine.hpp"
#include "scripting.hpp"

using namespace scripting;

static debug::Logger logger("scripting-hud");

Hud* scripting::hud = nullptr;
WorldRenderer* scripting::renderer = nullptr;

static void load_script(const std::string& name) {
    auto file = engine->getPaths().getResourcesFolder() / "scripts" / name;
    std::string src = files::read_string(file);
    logger.info() << "loading script " << file.u8string();

    lua::execute(lua::get_main_state(), 0, src, file.u8string());
}

void scripting::on_frontend_init(Hud* hud, WorldRenderer* renderer) {
    scripting::hud = hud;
    scripting::renderer = renderer;

    auto L = lua::get_main_state();

    lua::openlib(L, "hud", hudlib);
    lua::openlib(L, "gfx", "blockwraps", blockwrapslib);
    lua::openlib(L, "gfx", "particles", particleslib);
    lua::openlib(L, "gfx", "text3d", text3dlib);

    load_script("hud_classes.lua");

    if (lua::getglobal(L, "__vc_on_hud_open")) {
        lua::call_nothrow(L, 0, 0);
    }

    for (auto& pack : engine->getAllContentPacks()) {
        lua::emit_event(
            lua::get_main_state(),
            pack.id + ":.hudopen",
            [&](lua::State* L) {
                return lua::pushinteger(L, hud->getPlayer()->getId());
            }
        );
    }
}

void scripting::on_frontend_render() {
    for (auto& pack : engine->getAllContentPacks()) {
        lua::emit_event(
            lua::get_main_state(),
            pack.id + ":.hudrender",
            [](lua::State* L) { return 0; }
        );
    }
}

void scripting::on_frontend_close() {
    for (auto& pack : engine->getAllContentPacks()) {
        lua::emit_event(
            lua::get_main_state(),
            pack.id + ":.hudclose",
            [&](lua::State* L) {
                return lua::pushinteger(L, hud->getPlayer()->getId());
            }
        );
    }
    scripting::hud = nullptr;
}

void scripting::load_hud_script(
    const scriptenv& senv,
    const std::string& packid,
    const fs::path& file,
    const std::string& fileName
) {
    int env = *senv;
    std::string src = files::read_string(file);
    logger.info() << "loading script " << file.u8string();

    lua::execute(lua::get_main_state(), env, src, fileName);

    register_event(env, "init", packid + ":.init");
    register_event(env, "on_hud_open", packid + ":.hudopen");
    register_event(env, "on_hud_render", packid + ":.hudrender");
    register_event(env, "on_hud_close", packid + ":.hudclose");
}
