#include <iostream>
#include "Events.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>

inline constexpr short _MOUSE_KEYS_OFFSET = 1024;

bool Events::keys[KEYS_BUFFER_SIZE] = {};
uint Events::frames[KEYS_BUFFER_SIZE] = {};
uint Events::currentFrame = 0;
int Events::scroll = 0;
glm::vec2 Events::delta = {};
glm::vec2 Events::cursor = {};
bool Events::cursor_drag = false;
bool Events::_cursor_locked = false;
std::vector<uint> Events::codepoints;
std::vector<keycode> Events::pressedKeys;
std::unordered_map<std::string, Binding> Events::bindings;

bool Events::pressed(keycode keycode) {
    return pressed(static_cast<int>(keycode));
}

bool Events::pressed(int keycode) {
    if (keycode < 0 || keycode >= KEYS_BUFFER_SIZE) {
        return false;
    }
    return keys[keycode];
}

bool Events::jpressed(keycode keycode) {
    return jpressed(static_cast<int>(keycode));
}

bool Events::jpressed(int keycode) {
    return Events::pressed(keycode) && frames[keycode] == currentFrame;
}

bool Events::clicked(mousecode button) {
    return clicked(static_cast<int>(button));
}

bool Events::clicked(int button) {
    return Events::pressed(_MOUSE_KEYS_OFFSET + button);
}

bool Events::jclicked(mousecode button) {
    return jclicked(static_cast<int>(button));
}

bool Events::jclicked(int button) {
    return Events::jpressed(_MOUSE_KEYS_OFFSET + button);
}

void Events::toggleCursor() {
    _cursor_locked = !_cursor_locked;
    Window::setCursorMode(_cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Events::pollEvents() {
    currentFrame++;
    delta.x = 0.f;
    delta.y = 0.f;
    scroll = 0;
    codepoints.clear();
    pressedKeys.clear();
    glfwPollEvents();

    for (auto& entry : bindings) {
        auto& binding = entry.second;
        binding.justChange = false;

        bool newstate = false;
        switch (binding.type) {
            case inputtype::keyboard: newstate = pressed(binding.code); break;
            case inputtype::mouse: newstate = clicked(binding.code); break;
        }

        if (newstate) {
            if (!binding.state) {
                binding.state = true;
                binding.justChange = true;
            }
        }
        else {
            if (binding.state) {
                binding.state = false;
                binding.justChange = true;
            }
        }
    }
}

void Events::bind(std::string name, inputtype type, keycode code) {
    bind(name, type, static_cast<int>(code));
}

void Events::bind(std::string name, inputtype type, mousecode code) {
    bind(name, type, static_cast<int>(code));
}

void Events::bind(std::string name, inputtype type, int code) {
    bindings[name] = { type, code, false, false };
}

bool Events::active(std::string name) {
    const auto& found = bindings.find(name);
    if (found == bindings.end()) {
        return false;
    }
    return found->second.active();
}

bool Events::jactive(std::string name) {
    const auto& found = bindings.find(name);
    if (found == bindings.end()) {
        return false;
    }
    return found->second.jactive();
}

void Events::setKey(int key, bool b) {
    Events::keys[key] = b;
    Events::frames[key] = currentFrame;
}

void Events::setButton(int button, bool b) {
    setKey(_MOUSE_KEYS_OFFSET + button, b);
}

void Events::setPosition(float xpos, float ypos) {
    if (Events::cursor_drag) {
        Events::delta.x += xpos - Events::cursor.x;
        Events::delta.y += ypos - Events::cursor.y;
    }
    else {
        Events::cursor_drag = true;
    }
    Events::cursor.x = xpos;
    Events::cursor.y = ypos;
}
