#ifndef WINDOW_INPUT_H_
#define WINDOW_INPUT_H_

#include <string>

struct keycode {
    static const int ENTER;
    static const int TAB;
    static const int SPACE;
    static const int BACKSPACE;
    static const int LEFT_CONTROL;
    static const int LEFT_SHIFT;
    static const int LEFT_ALT;
    static const int RIGHT_CONTROL;
    static const int RIGHT_SHIFT;
    static const int RIGHT_ALT;
    static const int ESCAPE;
    static const int CAPS_LOCK;
    static const int LEFT;
    static const int RIGHT;
    static const int DOWN;
    static const int UP;
    static const int F1;
    static const int F2;
    static const int F3;
    static const int F4;
    static const int F5;
    static const int F6;
    static const int F7;
    static const int F8;
    static const int F9;
    static const int F10;
    static const int F11;
    static const int F12;
    static const int A;
    static const int B;
    static const int C;
    static const int D;
    static const int E;
    static const int F;
    static const int G;
    static const int H;
    static const int I;
    static const int J;
    static const int K;
    static const int L;
    static const int M;
    static const int N;
    static const int O;
    static const int P;
    static const int Q;
    static const int R;
    static const int S;
    static const int T;
    static const int U;
    static const int V;
    static const int W;
    static const int X;
    static const int Y;
    static const int Z;
    static const int NUM_0;
    static const int NUM_1;
    static const int NUM_2;
    static const int NUM_3;
    static const int NUM_4;
    static const int NUM_5;
    static const int NUM_6;
    static const int NUM_7;
    static const int NUM_8;
    static const int NUM_9;
    static const int MENU;
    static const int PAUSE;
    static const int INSERT;
    static const int LEFT_SUPER;
    static const int RIGHT_SUPER;
    static const int DELETE;
    static const int PAGE_UP;
    static const int PAGE_DOWN;
    static const int HOME;
    static const int END;
    static const int PRINT_SCREEN;
    static const int NUM_LOCK;
    static const int LEFT_BRACKET;
    static const int RIGHT_BRACKET;

    static const std::string name(int code);
};

struct mousecode {
    static int BUTTON_1;
    static int BUTTON_2;
    static int BUTTON_3;

    static const std::string name(int code);
};

enum class inputtype {
    keyboard,
    mouse,
};

struct Binding {
    inputtype type;
    int code;
    bool state = false;
    bool justChange = false;

    bool active() const {
        return state; 
    }

    bool jactive() const {
        return state && justChange;
    }

    const std::string text() const {
        switch (type) {
            case inputtype::keyboard: return keycode::name(code);
            case inputtype::mouse: return mousecode::name(code);
        }
        return "<unknown input type>";
    }
};


#endif // WINDOW_INPUT_H_