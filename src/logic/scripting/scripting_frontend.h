#ifndef LOGIC_SCRIPTING_SCRIPTING_FRONTEND_H_
#define LOGIC_SCRIPTING_SCRIPTING_FRONTEND_H_

class Hud;

namespace scripting {
    extern Hud* hud;

    void on_frontend_init(Hud* hud);
}

#endif // LOGIC_SCRIPTING_SCRIPTING_FRONTEND_H_
