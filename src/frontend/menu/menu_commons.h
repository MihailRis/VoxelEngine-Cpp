#ifndef FRONTEND_MENU_MENU_COMMONS_H_
#define FRONTEND_MENU_MENU_COMMONS_H_

#include <string>
#include <memory>
#include <glm/glm.hpp>

#include "../../graphics/ui/elements/controls.h"

class Engine;

namespace menus {
    extern std::shared_ptr<gui::Label> create_label(wstringsupplier supplier);
    extern std::shared_ptr<gui::Panel> create_page(
        Engine* engine, 
        std::string name, 
        int width, 
        float opacity, 
        int interval
    );
    extern std::shared_ptr<gui::Button> create_button(
        std::wstring text, 
        glm::vec4 padding, 
        glm::vec4 margin, 
        gui::onaction action
    );
}

#endif // FRONTEND_MENU_MENU_COMMONS_H_
