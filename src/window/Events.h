#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "Window.h"
#include "input.h"

#include <string>
#include <vector>
#include <unordered_map>

typedef unsigned int uint;

const short KEYS_BUFFER_SIZE = 1032;
const short _MOUSE_KEYS_OFFSET = 1024;

class Events {
public:
	static bool _keys[KEYS_BUFFER_SIZE];
	static uint _frames[KEYS_BUFFER_SIZE];
	static uint _current;
	static float deltaX;
	static float deltaY;
	static float x;
	static float y;
	static int scroll;
	static bool _cursor_locked;
	static bool _cursor_started;
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
};

#endif /* WINDOW_EVENTS_H_ */
