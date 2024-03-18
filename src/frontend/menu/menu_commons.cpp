#include "menu_commons.h"

#include "../../engine.h"
#include "../../graphics/ui/elements/containers.h"
#include "../locale/langs.h"

using namespace gui;

std::shared_ptr<Label> menus::create_label(wstringsupplier supplier) {
    auto label = std::make_shared<Label>(L"-");
    label->textSupplier(supplier);
    return label;
}

std::shared_ptr<Panel> menus::create_page(
    Engine* engine, 
    std::string name, 
    int width, 
    float opacity, 
    int interval
) {
    auto menu = engine->getGUI()->getMenu();
    auto panel = std::make_shared<Panel>(
        glm::vec2(width, 200), glm::vec4(8.0f), interval
    );
    panel->setColor(glm::vec4(0.0f, 0.0f, 0.0f, opacity));
    menu->addPage(name, panel);
    return panel;
}

std::shared_ptr<Button> menus::create_button(
    std::wstring text, 
    glm::vec4 padding, 
    glm::vec4 margin, 
    gui::onaction action
) {
    auto btn = std::make_shared<Button>(
        langs::get(text, L"menu"), padding, action
    );
    btn->setMargin(margin);
    return btn;
}
