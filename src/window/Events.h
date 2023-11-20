#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "Window.h"
#include "input.h"

#include <string>
#include <vector>
#include <unordered_map>

typedef unsigned int uint;

class Events {
public:
	static bool* _keys;
	static uint* _frames;
	static uint _current;
	static float deltaX;
	static float deltaY;
	static float x;
	static float y;
	static bool _cursor_locked;
	static bool _cursor_started;
	static std::vector<uint> codepoints;
	static std::vector<int> pressedKeys;
	static std::unordered_map<std::string, Binding> bindings;

	static int initialize();
	static void finalize();
	static void pullEvents();

	static bool pressed(int keycode);
	static bool jpressed(int keycode);

	static bool clicked(int button);
	static bool jclicked(int button);

	static void toggleCursor();

	static void bind(std::string name, inputtype type, int code);
	static bool active(std::string name);
	static bool jactive(std::string name);
};

#define _MOUSE_BUTTONS 1024

#endif /* WINDOW_EVENTS_H_ */
