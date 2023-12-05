#ifndef WINDOW_INPUT_H_
#define WINDOW_INPUT_H_

#include <string>

struct keycode {
    static int ENTER;
    static int TAB;
    static int SPACE;
    static int BACKSPACE;
    static int LEFT_CONTROL;
    static int LEFT_SHIFT;
    static int LEFT_ALT;
    static int RIGHT_CONTROL;
    static int RIGHT_SHIFT;
    static int RIGHT_ALT;
    static int ESCAPE;
    static int CAPS_LOCK;
    static int LEFT;
    static int RIGHT;
    static int DOWN;
    static int UP;
    static int F1;
    static int F2;
    static int F3;
    static int F4;
    static int F5;
    static int F6;
    static int F7;
    static int F8;
    static int F9;
    static int F10;
    static int F11;
    static int F12;
    static int A;
    static int B;
    static int C;
    static int D;
    static int E;
    static int F;
    static int G;
    static int H;
    static int I;
    static int J;
    static int K;
    static int L;
    static int M;
    static int N;
    static int O;
    static int P;
    static int Q;
    static int R;
    static int S;
    static int T;
    static int U;
    static int V;
    static int W;
    static int X;
    static int Y;
    static int Z;
    static int NUM_0;
    static int NUM_1;
    static int NUM_2;
    static int NUM_3;
    static int NUM_4;
    static int NUM_5;
    static int NUM_6;
    static int NUM_7;
    static int NUM_8;
    static int NUM_9;
    static int MENU;
    static int PAUSE;
    static int INSERT;
    static int LEFT_SUPER;
    static int RIGHT_SUPER;
    static int DELETE;
    static int PAGE_UP;
    static int PAGE_DOWN;
    static int HOME;
    static int END;
    static int PRINT_SCREEN;
    static int NUM_LOCK;
    static int LEFT_BRACKET;
    static int RIGHT_BRACKET;

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