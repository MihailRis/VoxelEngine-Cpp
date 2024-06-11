#include "scripting_hud.hpp"
#include "scripting.hpp"

#include "lua/lua_util.hpp"
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
using namespace scripting;

static debug::Logger logger("scripting-hud");

Hud* scripting::hud = nullptr;

void scripting::on_frontend_init(Hud* hud) {
    scripting::hud = hud;
    state->openlib(state->getMainThread(), "hud", hudlib);

    for (auto& pack : engine->getContentPacks()) {
        state->emitEvent(state->getMainThread(), pack.id + ".hudopen", 
        [&] (lua_State* L) {
            return lua::pushinteger(L, hud->getPlayer()->getId());        
        });
    }
}

void scripting::on_frontend_close() {
    for (auto& pack : engine->getContentPacks()) {
        state->emitEvent(state->getMainThread(), pack.id + ".hudclose", 
        [&] (lua_State* L) {
            return lua::pushinteger(L, hud->getPlayer()->getId());            
        });
    }
    scripting::hud = nullptr;
}

void scripting::load_hud_script(const scriptenv& senv, const std::string& packid, const fs::path& file) {
    int env = *senv;
    std::string src = files::read_string(file);
    logger.info() << "loading script " << file.u8string();

    lua::execute(state->getMainThread(), env, src, file.u8string());

    register_event(env, "init", packid+".init");
    register_event(env, "on_hud_open", packid+".hudopen");
    register_event(env, "on_hud_close", packid+".hudclose");
}
