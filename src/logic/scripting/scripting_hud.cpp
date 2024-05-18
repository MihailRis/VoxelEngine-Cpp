#include "scripting_hud.hpp"
#include "scripting.hpp"

#include "lua/api_lua.hpp"
#include "lua/LuaState.hpp"

#include "../../debug/Logger.hpp"
#include "../../frontend/hud.hpp"
#include "../../objects/Player.hpp"
#include "../../files/files.hpp"
#include "../../engine.hpp"

namespace scripting {
    extern lua::LuaState* state;
}

static debug::Logger logger("scripting-hud");

Hud* scripting::hud = nullptr;

void scripting::on_frontend_init(Hud* hud) {
    scripting::hud = hud;
    scripting::state->openlib("hud", hudlib);

    for (auto& pack : scripting::engine->getContentPacks()) {
        state->emit_event(pack.id + ".hudopen", [&] (lua::LuaState* state) {
            state->pushinteger(hud->getPlayer()->getId());
            return 1;            
        });
    }
}

void scripting::on_frontend_close() {
    for (auto& pack : scripting::engine->getContentPacks()) {
        state->emit_event(pack.id + ".hudclose", [&] (lua::LuaState* state) {
            state->pushinteger(hud->getPlayer()->getId());
            return 1;            
        });
    }
    scripting::hud = nullptr;
}

void scripting::load_hud_script(scriptenv senv, std::string packid, fs::path file) {
    int env = *senv;
    std::string src = files::read_string(file);
    logger.info() << "loading script " << file.u8string();

    state->loadbuffer(env, src, file.u8string());
    state->callNoThrow(0);

    register_event(env, "init", packid+".init");
    register_event(env, "on_hud_open", packid+".hudopen");
    register_event(env, "on_hud_close", packid+".hudclose");
}
