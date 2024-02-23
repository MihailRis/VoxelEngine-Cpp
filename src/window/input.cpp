#include "input.h"
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

void Binding::reset(inputtype type, int code) {
    this->type = type;
    this->code = code;
}

void Binding::reset(keycode code) {
    reset(inputtype::keyboard, static_cast<int>(code));
}

void Binding::reset(mousecode code) {
    reset(inputtype::mouse, static_cast<int>(code));
}

namespace input_util {

   std::string to_string(keycode code) {
#ifdef _WIN32
        char name[64];
        int result = GetKeyNameTextA(glfwGetKeyScancode(static_cast<int>(code)) << 16, name, 64);
        if (result == NULL) return "Unknown";
        return std::string(name);
#else
        const char* name = glfwGetKeyName(code, glfwGetKeyScancode(code));
        if (name == nullptr) {
            switch (code) {
            case GLFW_KEY_TAB: return "Tab";
            case GLFW_KEY_LEFT_CONTROL: return "Left Ctrl";
            case GLFW_KEY_RIGHT_CONTROL: return "Right Ctrl";
            case GLFW_KEY_LEFT_ALT: return "Left Alt";
            case GLFW_KEY_RIGHT_ALT: return "Right Alt";
            case GLFW_KEY_LEFT_SHIFT: return "Left Shift";
            case GLFW_KEY_RIGHT_SHIFT: return "Right Shift";
            case GLFW_KEY_CAPS_LOCK: return "Caps-Lock";
            case GLFW_KEY_SPACE: return "Space";
            case GLFW_KEY_ESCAPE: return "Esc";
            case GLFW_KEY_ENTER: return "Enter";
            case GLFW_KEY_UP: return "Up";
            case GLFW_KEY_DOWN: return "Down";
            case GLFW_KEY_LEFT: return "Left";
            case GLFW_KEY_RIGHT: return "Right";
            case GLFW_KEY_BACKSPACE: return "Backspace";
            case GLFW_KEY_F1: return "F1";
            case GLFW_KEY_F2: return "F2";
            case GLFW_KEY_F3: return "F3";
            case GLFW_KEY_F4: return "F4";
            case GLFW_KEY_F5: return "F5";
            case GLFW_KEY_F6: return "F6";
            case GLFW_KEY_F7: return "F7";
            case GLFW_KEY_F8: return "F8";
            case GLFW_KEY_F9: return "F9";
            case GLFW_KEY_F10: return "F10";
            case GLFW_KEY_F11: return "F11";
            case GLFW_KEY_F12: return "F12";
            case GLFW_KEY_DELETE: return "Delete";
            case GLFW_KEY_HOME: return "Home";
            case GLFW_KEY_END: return "End";
            case GLFW_KEY_LEFT_SUPER: return "Left Super";
            case GLFW_KEY_RIGHT_SUPER: return "Right Super";
            case GLFW_KEY_PAGE_UP: return "Page Up";
            case GLFW_KEY_PAGE_DOWN: return "Page Down";
            case GLFW_KEY_INSERT: return "Insert";
            case GLFW_KEY_PRINT_SCREEN: return "Print Screen";
            case GLFW_KEY_NUM_LOCK: return "Num Lock";
            case GLFW_KEY_MENU: return "Menu";
            case GLFW_KEY_PAUSE: return "Pause";
            default:
                return "Unknown";
            }
        }
        return std::string(name);
#endif // _WIN32
    }

    std::string to_string(mousecode code) {
        switch (code) {
        case mousecode::BUTTON_1: return "LMB";
        case mousecode::BUTTON_2: return "RMB";
        case mousecode::BUTTON_3: return "MMB";
        }
        return "unknown button";
    }

}