#include "libhud.h"

#include <iostream>

#include "../scripting.h"
#include "../../../frontend/hud.h"

namespace scripting {
    extern Hud* hud;
}

int l_hud_open_inventory(lua_State* L) {
    scripting::hud->openInventory();
    return 0;
}

int l_hud_close_inventory(lua_State* L) {
    scripting::hud->closeInventory();
    return 0;
}