#ifndef WINDOW_EVENTS_HPP_
#define WINDOW_EVENTS_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include <typedefs.hpp>
#include "input.hpp"

inline constexpr short KEYS_BUFFER_SIZE = 1036;

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

    static void setKey(int key, bool b);
    static void setButton(int button, bool b);

    static void setPosition(float xpos, float ypos);

    static std::string writeBindings();
    static void loadBindings(
        const std::string& filename, const std::string& source
    );
};

#endif  // WINDOW_EVENTS_HPP_
