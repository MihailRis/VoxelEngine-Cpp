#include "input.h"
#include <GLFW/glfw3.h>

const int keycode::ENTER = GLFW_KEY_ENTER;
const int keycode::TAB = GLFW_KEY_TAB;
const int keycode::SPACE = GLFW_KEY_SPACE;
const int keycode::BACKSPACE = GLFW_KEY_BACKSPACE;
const int keycode::LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT;
const int keycode::LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL;
const int keycode::LEFT_ALT = GLFW_KEY_LEFT_ALT;
const int keycode::RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT;
const int keycode::RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL;
const int keycode::RIGHT_ALT = GLFW_KEY_RIGHT_ALT;
const int keycode::ESCAPE = GLFW_KEY_ESCAPE;
const int keycode::CAPS_LOCK = GLFW_KEY_CAPS_LOCK;
const int keycode::LEFT = GLFW_KEY_LEFT;
const int keycode::RIGHT = GLFW_KEY_RIGHT;
const int keycode::DOWN = GLFW_KEY_DOWN;
const int keycode::UP = GLFW_KEY_UP;
const int keycode::F1 = GLFW_KEY_F1;
const int keycode::F2 = GLFW_KEY_F2;
const int keycode::F3 = GLFW_KEY_F3;
const int keycode::F4 = GLFW_KEY_F4;
const int keycode::F5 = GLFW_KEY_F5;
const int keycode::F6 = GLFW_KEY_F6;
const int keycode::F7 = GLFW_KEY_F7;
const int keycode::F8 = GLFW_KEY_F8;
const int keycode::F9 = GLFW_KEY_F9;
const int keycode::F10 = GLFW_KEY_F10;
const int keycode::F11 = GLFW_KEY_F11;
const int keycode::F12 = GLFW_KEY_F12;
const int keycode::A = GLFW_KEY_A;
const int keycode::B = GLFW_KEY_B;
const int keycode::C = GLFW_KEY_C;
const int keycode::D = GLFW_KEY_D;
const int keycode::E = GLFW_KEY_E;
const int keycode::F = GLFW_KEY_F;
const int keycode::G = GLFW_KEY_G;
const int keycode::H = GLFW_KEY_H;
const int keycode::I = GLFW_KEY_I;
const int keycode::J = GLFW_KEY_J;
const int keycode::K = GLFW_KEY_K;
const int keycode::L = GLFW_KEY_L;
const int keycode::M = GLFW_KEY_M;
const int keycode::N = GLFW_KEY_N;
const int keycode::O = GLFW_KEY_O;
const int keycode::P = GLFW_KEY_P;
const int keycode::Q = GLFW_KEY_Q;
const int keycode::R = GLFW_KEY_R;
const int keycode::S = GLFW_KEY_S;
const int keycode::T = GLFW_KEY_T;
const int keycode::U = GLFW_KEY_U;
const int keycode::V = GLFW_KEY_V;
const int keycode::W = GLFW_KEY_W;
const int keycode::X = GLFW_KEY_X;
const int keycode::Y = GLFW_KEY_Y;
const int keycode::Z = GLFW_KEY_Z;
const int keycode::NUM_0 = GLFW_KEY_0;
const int keycode::NUM_1 = GLFW_KEY_1;
const int keycode::NUM_2 = GLFW_KEY_2;
const int keycode::NUM_3 = GLFW_KEY_3;
const int keycode::NUM_4 = GLFW_KEY_4;
const int keycode::NUM_5 = GLFW_KEY_5;
const int keycode::NUM_6 = GLFW_KEY_6;
const int keycode::NUM_7 = GLFW_KEY_7;
const int keycode::NUM_8 = GLFW_KEY_8;
const int keycode::NUM_9 = GLFW_KEY_9;
const int keycode::MENU = GLFW_KEY_MENU;
const int keycode::PAUSE = GLFW_KEY_PAUSE;
const int keycode::INSERT = GLFW_KEY_INSERT;
const int keycode::LEFT_SUPER = GLFW_KEY_LEFT_SUPER;
const int keycode::RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER;
const int keycode::DELETE = GLFW_KEY_DELETE;
const int keycode::PAGE_UP = GLFW_KEY_PAGE_UP;
const int keycode::PAGE_DOWN = GLFW_KEY_PAGE_DOWN;
const int keycode::HOME = GLFW_KEY_HOME;
const int keycode::END = GLFW_KEY_END;
const int keycode::PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN;
const int keycode::NUM_LOCK = GLFW_KEY_NUM_LOCK;
const int keycode::LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET;
const int keycode::RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET;

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

const std::string keycode::name(int code) {
#ifdef _WIN32
    char name[64];
    int result = GetKeyNameTextA(glfwGetKeyScancode(code) << 16, name, 64);
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

int mousecode::BUTTON_1 = GLFW_MOUSE_BUTTON_1;
int mousecode::BUTTON_2 = GLFW_MOUSE_BUTTON_2;
int mousecode::BUTTON_3 = GLFW_MOUSE_BUTTON_3;

const std::string mousecode::name(int code) {
    switch (code) {
        case GLFW_MOUSE_BUTTON_1: return "LMB";
        case GLFW_MOUSE_BUTTON_2: return "RMB";
        case GLFW_MOUSE_BUTTON_3: return "MMB";
    }
    return "unknown button";
}
