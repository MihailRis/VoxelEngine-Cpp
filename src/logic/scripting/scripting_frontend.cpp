#include "scripting_frontend.h"
#include "scripting.h"

#include "lua/libhud.h"
#include "lua/LuaState.h"

namespace scripting {
    extern lua::LuaState* state;
}

Hud* scripting::hud = nullptr;

void scripting::on_frontend_init(Hud* hud) {
    scripting::hud = hud;
    scripting::state->openlib("hud", hudlib, 0);
}
