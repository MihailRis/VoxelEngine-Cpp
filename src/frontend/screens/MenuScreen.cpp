#include "MenuScreen.hpp"

#include "graphics/ui/GUI.hpp"
#include "graphics/ui/elements/Menu.hpp"
#include "graphics/core/Batch2D.hpp"
#include "graphics/core/Shader.hpp"
#include "graphics/core/Texture.hpp"
#include "maths/UVRegion.hpp"
#include "window/Window.hpp"
#include "window/Camera.hpp"
#include "engine.hpp"

MenuScreen::MenuScreen(Engine& engine) : Screen(engine) {
    engine.resetContent();
    
    auto menu = engine.getGUI()->getMenu();
    menu->reset();
    menu->setPage("main");

    uicamera = std::make_unique<Camera>(glm::vec3(), Window::height);
    uicamera->perspective = false;
    uicamera->flipped = true;
}

MenuScreen::~MenuScreen() {
}

void MenuScreen::update(float delta) {
}

void MenuScreen::draw(float delta) {
    auto assets = engine.getAssets();

    Window::clear();
    Window::setBgColor(glm::vec3(0.2f));

    uicamera->setFov(Window::height);
    auto uishader = assets->get<Shader>("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjView());

    uint width = Window::width;
    uint height = Window::height;

    auto bg = assets->get<Texture>("gui/menubg");
    batch->begin();
    batch->texture(bg);
    batch->rect(
        0, 0, 
        width, height, 0, 0, 0, 
        UVRegion(0, 0, width/bg->getWidth(), height/bg->getHeight()), 
        false, false, glm::vec4(1.0f)
    );
    batch->flush();
}
