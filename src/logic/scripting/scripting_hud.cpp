#include "scripting_hud.hpp"

#include <debug/Logger.hpp>
#include <engine.hpp>
#include <files/files.hpp>
#include <frontend/hud.hpp>
#include <objects/Player.hpp>
#include "lua/api_lua.hpp"
#include "lua/lua_engine.hpp"
#include "scripting.hpp"

using namespace scripting;

static debug::Logger logger("scripting-hud");

Hud* scripting::hud = nullptr;

void scripting::on_frontend_init(Hud* hud) {
    scripting::hud = hud;
    lua::openlib(lua::get_main_thread(), "hud", hudlib);

    for (auto& pack : engine->getContentPacks()) {
        lua::emit_event(
            lua::get_main_thread(),
            pack.id + ".hudopen",
            [&](lua::State* L) {
                return lua::pushinteger(L, hud->getPlayer()->getId());
            }
        );
    }
}

void scripting::on_frontend_render() {
    for (auto& pack : engine->getContentPacks()) {
        lua::emit_event(
            lua::get_main_thread(),
            pack.id + ".hudrender",
            [&](lua::State* L) { return 0; }
        );
    }
}

void scripting::on_frontend_close() {
    for (auto& pack : engine->getContentPacks()) {
        lua::emit_event(
            lua::get_main_thread(),
            pack.id + ".hudclose",
            [&](lua::State* L) {
                return lua::pushinteger(L, hud->getPlayer()->getId());
            }
        );
    }
    scripting::hud = nullptr;
}

void scripting::load_hud_script(
    const scriptenv& senv, const std::string& packid, const fs::path& file
) {
    int env = *senv;
    std::string src = files::read_string(file);
    logger.info() << "loading script " << file.u8string();

    lua::execute(lua::get_main_thread(), env, src, file.u8string());

    register_event(env, "init", packid + ".init");
    register_event(env, "on_hud_open", packid + ".hudopen");
    register_event(env, "on_hud_render", packid + ".hudrender");
    register_event(env, "on_hud_close", packid + ".hudclose");
}
