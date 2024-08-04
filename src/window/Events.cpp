#include "Events.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>

#include "../debug/Logger.hpp"
#include "../util/stringutil.hpp"
#include "Window.hpp"

static debug::Logger logger("events");

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
    cursor_drag = false;
    _cursor_locked = !_cursor_locked;
    Window::setCursorMode(
        _cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
    );
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
            case inputtype::keyboard:
                newstate = pressed(binding.code);
                break;
            case inputtype::mouse:
                newstate = clicked(binding.code);
                break;
        }

        if (newstate) {
            if (!binding.state) {
                binding.state = true;
                binding.justChange = true;
                binding.onactived.notify();
            }
        } else {
            if (binding.state) {
                binding.state = false;
                binding.justChange = true;
            }
        }
    }
}

Binding& Events::getBinding(const std::string& name) {
    auto found = bindings.find(name);
    if (found == bindings.end()) {
        throw std::runtime_error("binding '" + name + "' does not exists");
    }
    return found->second;
}

void Events::bind(const std::string& name, inputtype type, keycode code) {
    bind(name, type, static_cast<int>(code));
}

void Events::bind(const std::string& name, inputtype type, mousecode code) {
    bind(name, type, static_cast<int>(code));
}

void Events::bind(const std::string& name, inputtype type, int code) {
    bindings.try_emplace(name, Binding(type, code));
}

void Events::rebind(const std::string& name, inputtype type, int code) {
    bindings[name] = Binding(type, code);
}

bool Events::active(const std::string& name) {
    const auto& found = bindings.find(name);
    if (found == bindings.end()) {
        return false;
    }
    return found->second.active();
}

bool Events::jactive(const std::string& name) {
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
    } else {
        Events::cursor_drag = true;
    }
    Events::cursor.x = xpos;
    Events::cursor.y = ypos;
}

#include "../coders/json.hpp"
#include "../coders/toml.hpp"
#include "../data/dynamic.hpp"

std::string Events::writeBindings() {
    dynamic::Map obj;
    for (auto& entry : Events::bindings) {
        const auto& binding = entry.second;
        std::string value;
        switch (binding.type) {
            case inputtype::keyboard:
                value =
                    "key:" +
                    input_util::get_name(static_cast<keycode>(binding.code));
                break;
            case inputtype::mouse:
                value =
                    "mouse:" +
                    input_util::get_name(static_cast<mousecode>(binding.code));
                break;
            default:
                throw std::runtime_error("unsupported control type");
        }
        obj.put(entry.first, value);
    }
    return toml::stringify(obj);
}

void Events::loadBindings(
    const std::string& filename, const std::string& source
) {
    auto map = toml::parse(filename, source);
    for (auto& entry : map->values) {
        if (auto value = std::get_if<std::string>(&entry.second)) {
            auto [prefix, codename] = util::split_at(*value, ':');
            inputtype type;
            int code;
            if (prefix == "key") {
                type = inputtype::keyboard;
                code = static_cast<int>(input_util::keycode_from(codename));
            } else if (prefix == "mouse") {
                type = inputtype::mouse;
                code = static_cast<int>(input_util::mousecode_from(codename));
            } else {
                logger.error()
                    << "unknown input type: " << prefix << " (binding "
                    << util::quote(entry.first) << ")";
                continue;
            }
            Events::bind(entry.first, type, code);
        } else {
            logger.error() << "invalid binding entry: " << entry.first;
        }
    }
}
