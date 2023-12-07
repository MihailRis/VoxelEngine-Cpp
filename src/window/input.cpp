#include "input.h"
#include <GLFW/glfw3.h>

int keycode::ENTER = GLFW_KEY_ENTER;
int keycode::TAB = GLFW_KEY_TAB;
int keycode::SPACE = GLFW_KEY_SPACE;
int keycode::BACKSPACE = GLFW_KEY_BACKSPACE;
int keycode::LEFT_SHIFT = GLFW_KEY_LEFT_SHIFT;
int keycode::LEFT_CONTROL = GLFW_KEY_LEFT_CONTROL;
int keycode::LEFT_ALT = GLFW_KEY_LEFT_ALT;
int keycode::RIGHT_SHIFT = GLFW_KEY_RIGHT_SHIFT;
int keycode::RIGHT_CONTROL = GLFW_KEY_RIGHT_CONTROL;
int keycode::RIGHT_ALT = GLFW_KEY_RIGHT_ALT;
int keycode::ESCAPE = GLFW_KEY_ESCAPE;
int keycode::CAPS_LOCK = GLFW_KEY_CAPS_LOCK;
int keycode::LEFT = GLFW_KEY_LEFT;
int keycode::RIGHT = GLFW_KEY_RIGHT;
int keycode::DOWN = GLFW_KEY_DOWN;
int keycode::UP = GLFW_KEY_UP;
int keycode::F1 = GLFW_KEY_F1;
int keycode::F2 = GLFW_KEY_F2;
int keycode::F3 = GLFW_KEY_F3;
int keycode::F4 = GLFW_KEY_F4;
int keycode::F5 = GLFW_KEY_F5;
int keycode::F6 = GLFW_KEY_F6;
int keycode::F7 = GLFW_KEY_F7;
int keycode::F8 = GLFW_KEY_F8;
int keycode::F9 = GLFW_KEY_F9;
int keycode::F10 = GLFW_KEY_F10;
int keycode::F11 = GLFW_KEY_F11;
int keycode::F12 = GLFW_KEY_F12;
int keycode::A = GLFW_KEY_A;
int keycode::B = GLFW_KEY_B;
int keycode::C = GLFW_KEY_C;
int keycode::D = GLFW_KEY_D;
int keycode::E = GLFW_KEY_E;
int keycode::F = GLFW_KEY_F;
int keycode::G = GLFW_KEY_G;
int keycode::H = GLFW_KEY_H;
int keycode::I = GLFW_KEY_I;
int keycode::J = GLFW_KEY_J;
int keycode::K = GLFW_KEY_K;
int keycode::L = GLFW_KEY_L;
int keycode::M = GLFW_KEY_M;
int keycode::N = GLFW_KEY_N;
int keycode::O = GLFW_KEY_O;
int keycode::P = GLFW_KEY_P;
int keycode::Q = GLFW_KEY_Q;
int keycode::R = GLFW_KEY_R;
int keycode::S = GLFW_KEY_S;
int keycode::T = GLFW_KEY_T;
int keycode::U = GLFW_KEY_U;
int keycode::V = GLFW_KEY_V;
int keycode::W = GLFW_KEY_W;
int keycode::X = GLFW_KEY_X;
int keycode::Y = GLFW_KEY_Y;
int keycode::Z = GLFW_KEY_Z;
int keycode::NUM_0 = GLFW_KEY_0;
int keycode::NUM_1 = GLFW_KEY_1;
int keycode::NUM_2 = GLFW_KEY_2;
int keycode::NUM_3 = GLFW_KEY_3;
int keycode::NUM_4 = GLFW_KEY_4;
int keycode::NUM_5 = GLFW_KEY_5;
int keycode::NUM_6 = GLFW_KEY_6;
int keycode::NUM_7 = GLFW_KEY_7;
int keycode::NUM_8 = GLFW_KEY_8;
int keycode::NUM_9 = GLFW_KEY_9;
int keycode::MENU = GLFW_KEY_MENU;
int keycode::PAUSE = GLFW_KEY_PAUSE;
int keycode::INSERT = GLFW_KEY_INSERT;
int keycode::LEFT_SUPER = GLFW_KEY_LEFT_SUPER;
int keycode::RIGHT_SUPER = GLFW_KEY_RIGHT_SUPER;
int keycode::DELETE = GLFW_KEY_DELETE;
int keycode::PAGE_UP = GLFW_KEY_PAGE_UP;
int keycode::PAGE_DOWN = GLFW_KEY_PAGE_DOWN;
int keycode::HOME = GLFW_KEY_HOME;
int keycode::END = GLFW_KEY_END;
int keycode::PRINT_SCREEN = GLFW_KEY_PRINT_SCREEN;
int keycode::NUM_LOCK = GLFW_KEY_NUM_LOCK;
int keycode::LEFT_BRACKET = GLFW_KEY_LEFT_BRACKET;
int keycode::RIGHT_BRACKET = GLFW_KEY_RIGHT_BRACKET;

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

std::string keycode::name(int code) {
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

std::string mousecode::name(int code) {
    switch (code) {
        case GLFW_MOUSE_BUTTON_1: return "LMB";
        case GLFW_MOUSE_BUTTON_2: return "RMB";
        case GLFW_MOUSE_BUTTON_3: return "MMB";
    }
    return "unknown button";
}
