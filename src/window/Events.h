#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "Window.h"
#include "input.h"

#include <string>
#include <vector>
#include <unordered_map>

typedef unsigned int uint;

const short KEYS_BUFFER_SIZE = 1036;
const short _MOUSE_KEYS_OFFSET = 1024;

class Events {
public:
	static bool _keys[KEYS_BUFFER_SIZE];
	static uint _frames[KEYS_BUFFER_SIZE];
	static uint _current;
    static int scroll;
    static glm::vec2 delta;
    static glm::vec2 cursor;
    static bool cursor_drag;
	static bool _cursor_locked;
	static std::vector<uint> codepoints;
	static std::vector<int> pressedKeys;
	static std::unordered_map<std::string, Binding> bindings;

	static void pollEvents();

	static bool pressed(int keycode);
	static bool jpressed(int keycode);

	static bool clicked(int button);
	static bool jclicked(int button);

	static void toggleCursor();

	static void bind(std::string name, inputtype type, int code);
	static bool active(std::string name);
	static bool jactive(std::string name);

    static void setKey(int key, bool b);
    static void setButton(int button, bool b);

    static void setPosition(float xpos, float ypos);
};

#endif /* WINDOW_EVENTS_H_ */
