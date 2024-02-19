#include "scripting_frontend.h"
#include "scripting.h"

#include "lua/libhud.h"
#include "lua/LuaState.h"

#include "../../frontend/hud.h"
#include "../../objects/Player.h"
#include "../../files/files.h"
#include "../../engine.h"

#include <iostream>

namespace scripting {
    extern lua::LuaState* state;
}

Hud* scripting::hud = nullptr;

void scripting::on_frontend_init(Hud* hud) {
    scripting::hud = hud;
    scripting::state->openlib("hud", hudlib, 0);

    for (auto& pack : scripting::engine->getContentPacks()) {
        if (state->getglobal(pack.id+".hudopen")) {
            state->pushinteger(hud->getPlayer()->getId());
            state->callNoThrow(1);
        }
    }
}

void scripting::on_frontend_close() {
    scripting::hud = nullptr;
    for (auto& pack : scripting::engine->getContentPacks()) {
        if (state->getglobal(pack.id+".hudclose")) {
            state->pushinteger(hud->getPlayer()->getId());
            state->callNoThrow(1);
        }
    }
}

void scripting::load_hud_script(int env, std::string packid, fs::path file) {
    std::string src = files::read_string(file);
    std::cout << "loading script " << file.u8string() << std::endl;

    state->loadbuffer(env, src, file.u8string());
    state->callNoThrow(0);

    register_event(env, "init", packid+".init");
    register_event(env, "on_hud_open", packid+".hudopen");
    register_event(env, "on_hud_close", packid+".hudclose");
}
