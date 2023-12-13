#ifndef FRONTEND_MENU_H_
#define FRONTEND_MENU_H_

class Engine;

namespace gui {
    class PagesControl;
}

namespace menus {
    void create_menus(Engine* engine, gui::PagesControl* menu);
    void refresh_menus(Engine* engine, gui::PagesControl* menu);
}

#endif // FRONTEND_MENU_H_