#include "GUI.h"
#include "UINode.h"
#include "panels.h"

#include <iostream>

#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../window/Events.h"
#include "../../window/input.h"

using std::shared_ptr;
using namespace gui;

GUI::GUI() {
    container = new Container(vec2(0, 0), vec2(Window::width, Window::height));
}

GUI::~GUI() {
    delete container;
}

void GUI::act(float delta) {
    for (IntervalEvent& event : intervalEvents) {
        event.timer += delta;
        if (event.timer > event.interval) {
            event.callback();
            event.timer = fmod(event.timer, event.interval);
        }
    }

    container->size(vec2(Window::width, Window::height));
    int mx = Events::x;
    int my = Events::y;

    auto hover = container->getAt(vec2(mx, my), nullptr);
    if (this->hover && this->hover != hover) {
        this->hover->hover(false);
    }
    if (hover) {
        hover->hover(true);
    }
    this->hover = hover;

    if (Events::clicked(0)) {
        if (pressed == nullptr && this->hover) {
            pressed = hover;
            pressed->click(this, mx, my);
            if (focus) {
                focus->defocus();
            }
            focus = pressed;
        }
        if (this->hover == nullptr && focus) {
            focus->defocus();
            focus = nullptr;
        }
    } else if (pressed) {
        pressed->mouseRelease(this, mx, my);
        pressed = nullptr;
    }
    if (focus) {
        if (!focus->isfocused()){
            focus = nullptr;
        } else if (Events::jpressed(keycode::ESCAPE)) {
            focus->defocus();
            focus = nullptr;
        } else {
            for (auto codepoint : Events::codepoints) {
                focus->typed(codepoint);
            }
            for (auto key : Events::pressedKeys) {
                focus->keyPressed(key);
            }
        }
    }
}

void GUI::draw(Batch2D* batch, Assets* assets) {
    container->draw(batch, assets);
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

void GUI::interval(float interval, ontimeout callback) {
    intervalEvents.push_back({callback, interval, 0.0f});
}