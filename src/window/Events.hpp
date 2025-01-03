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

class Events {
    static bool keys[KEYS_BUFFER_SIZE];
    static uint frames[KEYS_BUFFER_SIZE];
    static uint currentFrame;
    static bool cursor_drag;
public:
    static int scroll;
    static glm::vec2 delta;
    static glm::vec2 cursor;
    static bool _cursor_locked;
    static std::vector<uint> codepoints;
    static std::vector<keycode> pressedKeys;
    static std::unordered_map<std::string, Binding> bindings;
    static std::unordered_map<keycode, util::HandlersList<>> keyCallbacks;

    static void pollEvents();

    static bool pressed(keycode keycode);
    static bool pressed(int keycode);
    static bool jpressed(keycode keycode);
    static bool jpressed(int keycode);

    static bool clicked(mousecode button);
    static bool clicked(int button);
    static bool jclicked(mousecode button);
    static bool jclicked(int button);

    static void toggleCursor();

    static Binding& getBinding(const std::string& name);
    static void bind(const std::string& name, inputtype type, keycode code);
    static void bind(const std::string& name, inputtype type, mousecode code);
    static void bind(const std::string& name, inputtype type, int code);
    static void rebind(const std::string& name, inputtype type, int code);
    static bool active(const std::string& name);
    static bool jactive(const std::string& name);

    static observer_handler addKeyCallback(keycode key, KeyCallback callback);

    static void setKey(int key, bool b);
    static void setButton(int button, bool b);

    static void setPosition(float xpos, float ypos);

    static std::string writeBindings();
    static void loadBindings(
        const std::string& filename, const std::string& source,
        BindType bindType
    );
    static void enableBindings();
};
