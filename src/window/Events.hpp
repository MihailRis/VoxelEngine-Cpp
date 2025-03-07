#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "delegates.hpp"
#include "typedefs.hpp"
#include "input.hpp"

inline constexpr short KEYS_BUFFER_SIZE = 1036;

enum class BindType {
    BIND = 0,
    REBIND = 1
};

namespace Events {
    extern int scroll;
    extern glm::vec2 delta;
    extern glm::vec2 cursor;
    extern std::vector<uint> codepoints;
    extern std::vector<keycode> pressedKeys;
    extern Bindings bindings;

    void pollEvents();

    int getScroll();

    bool pressed(keycode keycode);
    bool pressed(int keycode);
    bool jpressed(keycode keycode);
    bool jpressed(int keycode);

    bool clicked(mousecode button);
    bool clicked(int button);
    bool jclicked(mousecode button);
    bool jclicked(int button);

    void toggleCursor();

    Binding* getBinding(const std::string& name);
    Binding& requireBinding(const std::string& name);
    void bind(const std::string& name, inputtype type, keycode code);
    void bind(const std::string& name, inputtype type, mousecode code);
    void bind(const std::string& name, inputtype type, int code);
    void rebind(const std::string& name, inputtype type, int code);
    bool active(const std::string& name);
    bool jactive(const std::string& name);

    observer_handler addKeyCallback(keycode key, KeyCallback callback);

    void setKey(int key, bool b);
    void setButton(int button, bool b);

    void setPosition(float xpos, float ypos);

    std::string writeBindings();
    void loadBindings(
        const std::string& filename,
        const std::string& source,
        BindType bindType
    );
    void enableBindings();

    bool isCursorLocked();
};
