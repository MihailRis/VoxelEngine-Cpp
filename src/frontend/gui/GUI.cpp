#include "GUI.h"
#include "UINode.h"
#include "panels.h"

#include <iostream>
#include <algorithm>

#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics-vk/Batch2D.h"
#include "../../graphics/Shader.h"
#include "../../window/Events.h"
#include "../../window/input.h"
#include "../../window/Camera.h"

using glm::vec2;
using glm::vec3;
using std::string;
using std::shared_ptr;
using namespace gui;

GUI::GUI() {
    container = new Container(vec2(0, 0), vec2(1000));

    const vec3 camPos = vulkan::VulkanContext::isVulkanEnabled() ? vec3(0, 0, -1) : vec3();
    uicamera = new Camera(camPos, static_cast<float>(Window::height));
	uicamera->perspective = false;
	uicamera->flipped = true;

    menu = new PagesControl();
    container->add(menu);
    container->scrollable(false);
}

GUI::~GUI() {
    delete uicamera;
    delete container;
}

PagesControl* GUI::getMenu() {
    return menu;
}

void GUI::actMouse(float delta) {
    int mx = Events::x;
    int my = Events::y;

    auto hover = container->getAt(vec2(mx, my), nullptr);
    if (this->hover && this->hover != hover) {
        this->hover->hover(false);
    }
    if (hover) {
        hover->hover(true);
        if (Events::scroll) {
            hover->scrolled(Events::scroll);
        }
    }
    this->hover = hover;

    if (Events::jclicked(0)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            pressed->click(this, mx, my);
            if (focus && focus != pressed) {
                focus->defocus();
            }
            if (focus != pressed) {
                focus = pressed;
                focus->focus(this);
            }
        }
        if (this->hover == nullptr && focus) {
            focus->defocus();
            focus = nullptr;
        }
    } else if (pressed) {
        pressed->mouseRelease(this, mx, my);
        pressed = nullptr;
    }
} 

void GUI::act(float delta) {
    container->size(vec2(Window::width, Window::height));
    container->act(delta);
    auto prevfocus = focus;

    if (!Events::_cursor_locked) {
        actMouse(delta);
    }
    
    if (focus) {
        if (Events::jpressed(keycode::ESCAPE)) {
            focus->defocus();
            focus = nullptr;
        } else {
            for (auto codepoint : Events::codepoints) {
                focus->typed(codepoint);
            }
            for (auto key : Events::pressedKeys) {
                focus->keyPressed(key);
            }

            if (!Events::_cursor_locked) {
                if (Events::clicked(mousecode::BUTTON_1)) {
                    int mx = Events::x;
                    int my = Events::y;
                    focus->mouseMove(this, mx, my);
                }
                if (prevfocus == focus){
                    for (int i = mousecode::BUTTON_1; i < mousecode::BUTTON_1+12; i++) {
                        if (Events::jclicked(i)) {
                            focus->clicked(this, i);
                        }
                    }
                }
            }
        }
    }
    if (focus && !focus->isfocused()) {
        focus = nullptr;
    }
}

void GUI::draw(vulkan::Batch2D* batch, Assets* assets) {
    vulkan::VulkanContext::get().beginGuiDraw();
    menu->setCoord((Window::size() - menu->size()) / 2.0f);
    uicamera->setFov(Window::height);

	IShader* uishader = assets->getShader("ui");
	uishader->use();
	uishader->uniformMatrix("u_projview", uicamera->getProjection()*uicamera->getView());

    batch->begin();
    container->draw(batch, assets);
    batch->end();

    vulkan::VulkanContext::get().endGuiDraw();
}

shared_ptr<UINode> GUI::getFocused() const {
    return focus;
}

bool GUI::isFocusCaught() const {
    return focus && focus->isfocuskeeper();
}

void GUI::add(shared_ptr<UINode> panel) {
    container->add(panel);
}

void GUI::remove(shared_ptr<UINode> panel) {
    container->remove(panel);
}

void GUI::store(string name, shared_ptr<UINode> node) {
    storage[name] = node;
}

shared_ptr<UINode> GUI::get(string name) {
    auto found = storage.find(name);
    if (found == storage.end()) {
        return nullptr;
    }
    return found->second;
}

void GUI::remove(string name) {
    storage.erase(name);
}

void GUI::setFocus(shared_ptr<UINode> node) {
    if (focus) {
        focus->defocus();
    }
    focus = node;
    if (focus) {
        focus->focus(this);
    }
}
