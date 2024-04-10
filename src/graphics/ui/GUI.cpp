#include "GUI.h"
#include "elements/UINode.h"
#include "elements/containers.h"

#include <iostream>
#include <algorithm>

#include "../../assets/Assets.h"
#include "../../frontend/UiDocument.h"
#include "../../graphics/core/Batch2D.h"
#include "../../graphics/core/Shader.h"
#include "../../graphics/core/GfxContext.h"
#include "../../window/Events.h"
#include "../../window/input.h"
#include "../../window/Camera.h"

using namespace gui;

GUI::GUI() {
    container = std::make_shared<Container>(glm::vec2(1000));
    uicamera = std::make_unique<Camera>(glm::vec3(), Window::height);
    uicamera->perspective = false;
    uicamera->flipped = true;

    menu = std::make_shared<Menu>();
    menu->setId("menu");
    container->add(menu);
    container->setScrollable(false);
}

GUI::~GUI() {
}

std::shared_ptr<Menu> GUI::getMenu() {
    return menu;
}

void GUI::onAssetsLoad(Assets* assets) {
    assets->store(new UiDocument(
        "core:root", 
        uidocscript {}, 
        std::dynamic_pointer_cast<gui::UINode>(container), 
        nullptr
    ), "core:root");
}

/** Mouse related input and logic handling 
 * @param delta delta time
*/
void GUI::actMouse(float delta) {
    auto hover = container->getAt(Events::cursor, nullptr);
    if (this->hover && this->hover != hover) {
        this->hover->setHover(false);
    }
    if (hover) {
        hover->setHover(true);
        if (Events::scroll) {
            hover->scrolled(Events::scroll);
        }
    }
    this->hover = hover;

    if (Events::jclicked(mousecode::BUTTON_1)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            pressed->click(this, Events::cursor.x, Events::cursor.y);
            if (focus && focus != pressed) {
                focus->defocus();
            }
            if (focus != pressed) {
                focus = pressed;
                focus->onFocus(this);
                return;
            }
        }
        if (this->hover == nullptr && focus) {
            focus->defocus();
            focus = nullptr;
        }
    } else if (pressed) {
        pressed->mouseRelease(this, Events::cursor.x, Events::cursor.y);
        pressed = nullptr;
    }

    if (hover) {
        for (mousecode code : MOUSECODES_ALL) {
            if (Events::jclicked(code)) {
                hover->clicked(this, code);
            }
        }
    }
} 

void GUI::actFocused() {
    if (Events::jpressed(keycode::ESCAPE)) {
        focus->defocus();
        focus = nullptr;
        return;
    }
    for (auto codepoint : Events::codepoints) {
        focus->typed(codepoint);
    }
    for (auto key : Events::pressedKeys) {
        focus->keyPressed(key);
    }

    if (!Events::_cursor_locked) {
        if (Events::clicked(mousecode::BUTTON_1) && 
            (Events::jclicked(mousecode::BUTTON_1) || Events::delta.x || Events::delta.y))
        {
            focus->mouseMove(this, Events::cursor.x, Events::cursor.y);
        }
    }
}

/// @brief Processing user input and UI logic 
/// @param delta delta time
void GUI::act(float delta) {
    while (!postRunnables.empty()) {
        runnable callback = postRunnables.back();
        postRunnables.pop();
        callback();
    }

    container->setSize(glm::vec2(Window::width, Window::height));
    container->act(delta);
    auto prevfocus = focus;

    if (!Events::_cursor_locked) {
        actMouse(delta);
    }
    
    if (focus) {
        actFocused();
    }
    if (focus && !focus->isFocused()) {
        focus = nullptr;
    }
}

void GUI::draw(const GfxContext* pctx, Assets* assets) {
    auto& viewport = pctx->getViewport();
    glm::vec2 wsize = viewport.size();

    menu->setPos((wsize - menu->getSize()) / 2.0f);
    uicamera->setFov(wsize.y);

    Shader* uishader = assets->getShader("ui");
    uishader->use();
    uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

    pctx->getBatch2D()->begin();
    container->draw(pctx, assets);
}

std::shared_ptr<UINode> GUI::getFocused() const {
    return focus;
}

bool GUI::isFocusCaught() const {
    return focus && focus->isFocuskeeper();
}

void GUI::add(std::shared_ptr<UINode> node) {
    container->add(node);
}

void GUI::remove(std::shared_ptr<UINode> node) noexcept {
    container->remove(node);
}

void GUI::store(std::string name, std::shared_ptr<UINode> node) {
    storage[name] = node;
}

std::shared_ptr<UINode> GUI::get(std::string name) noexcept {
    auto found = storage.find(name);
    if (found == storage.end()) {
        return nullptr;
    }
    return found->second;
}

void GUI::remove(std::string name) noexcept {
    storage.erase(name);
}

void GUI::setFocus(std::shared_ptr<UINode> node) {
    if (focus) {
        focus->defocus();
    }
    focus = node;
    if (focus) {
        focus->onFocus(this);
    }
}

std::shared_ptr<Container> GUI::getContainer() const {
    return container;
}

void GUI::postRunnable(runnable callback) {
    postRunnables.push(callback);
}
