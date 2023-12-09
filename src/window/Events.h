#ifndef WINDOW_EVENTS_H_
#define WINDOW_EVENTS_H_

#include "Window.h"
#include "input.h"

#include <string>
#include <vector>
#include <unordered_map>

typedef unsigned int uint;

extern const short KEYS_BUFFER_SIZE;
extern const short _MOUSE_KEYS_OFFSET;

class Events {
public:
	static bool* _keys;
	static uint* _frames;
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

	static int initialize();
	static void finalize();
	static void pollEvents();

	static bool pressed(int keycode);
	static bool jpressed(int keycode);

	static bool clicked(int button);
	static bool jclicked(int button);

	static void toggleCursor();

	static void bind(const std::string& name, inputtype type, int code);
	static bool active(const std::string& name);
	static bool jactive(const std::string& name);
};

#endif /* WINDOW_EVENTS_H_ */
