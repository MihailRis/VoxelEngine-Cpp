#ifndef WINDOW_INPUT_H_
#define WINDOW_INPUT_H_

namespace keycode {
    extern int ENTER;
    extern int TAB;
    extern int SPACE;
    extern int BACKSPACE;
    extern int LEFT_CONTROL;
    extern int LEFT_SHIFT;
    extern int LEFT_ALT;
    extern int RIGHT_CONTROL;
    extern int RIGHT_SHIFT;
    extern int RIGHT_ALT;
    extern int ESCAPE;
    extern int CAPS_LOCK;
    extern int LEFT;
    extern int RIGHT;
    extern int DOWN;
    extern int UP;
    extern int F1;
    extern int F2;
    extern int F3;
    extern int F4;
    extern int F5;
    extern int F6;
    extern int F7;
    extern int F8;
    extern int F9;
    extern int F10;
    extern int F11;
    extern int F12;
    extern int A;
    extern int B;
    extern int C;
    extern int D;
    extern int E;
    extern int F;
    extern int G;
    extern int H;
    extern int I;
    extern int J;
    extern int K;
    extern int L;
    extern int M;
    extern int N;
    extern int O;
    extern int P;
    extern int Q;
    extern int R;
    extern int S;
    extern int T;
    extern int U;
    extern int V;
    extern int W;
    extern int X;
    extern int Y;
    extern int Z;
    extern int NUM_0;
    extern int NUM_1;
    extern int NUM_2;
    extern int NUM_3;
    extern int NUM_4;
    extern int NUM_5;
    extern int NUM_6;
    extern int NUM_7;
    extern int NUM_8;
    extern int NUM_9;

    extern const char* name(int code);
}

namespace mousecode {
    extern int BUTTON_1;
    extern int BUTTON_2;
    extern int BUTTON_3;

    extern const char* name(int code);
}

enum class inputtype {
    keyboard,
    button,
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

    const char* text() const {
        switch (type) {
            case inputtype::keyboard: return keycode::name(code);
            case inputtype::button: return mousecode::name(code);
        }
        return "<unknown input type>";
    }
};


#endif // WINDOW_INPUT_H_