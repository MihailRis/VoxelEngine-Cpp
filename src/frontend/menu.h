#ifndef FRONTEND_MENU_H_
#define FRONTEND_MENU_H_

class Engine;
class Level;

namespace menus {
    void create_version_label(Engine* engine);
    void create_pause_panel(Engine* engine, Level* level);
    void create_menus(Engine* engine);
    void refresh_menus(Engine* engine);
}

#endif // FRONTEND_MENU_H_