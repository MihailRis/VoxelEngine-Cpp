#include "Events.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>

bool* Events::_keys;
uint* Events::_frames;
uint Events::_current = 0;
float Events::deltaX = 0.0f;
float Events::deltaY = 0.0f;
float Events::x = 0.0f;
float Events::y = 0.0f;
bool Events::_cursor_locked = false;
bool Events::_cursor_started = false;
std::vector<uint> Events::codepoints;
std::vector<int> Events::pressedKeys;
std::unordered_map<std::string, Binding> Events::bindings;

int Events::initialize(){
	_keys = new bool[1032];
	_frames = new uint[1032];

	memset(_keys, false, 1032*sizeof(bool));
	memset(_frames, 0, 1032*sizeof(uint));

	return 0;
}

void Events::finalize(){
	delete[] _keys;
	delete[] _frames;
}

bool Events::pressed(int keycode){
	if (keycode < 0 || keycode >= _MOUSE_BUTTONS)
		return false;
	return _keys[keycode];
}

bool Events::jpressed(int keycode){
	if (keycode < 0 || keycode >= _MOUSE_BUTTONS)
		return false;
	return _keys[keycode] && _frames[keycode] == _current;
}

bool Events::clicked(int button){
	int index = _MOUSE_BUTTONS+button;
	return _keys[index];
}

bool Events::jclicked(int button){
	int index = _MOUSE_BUTTONS+button;
	return _keys[index] && _frames[index] == _current;
}

void Events::toggleCursor(){
	_cursor_locked = !_cursor_locked;
	Window::setCursorMode(_cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Events::pullEvents(){
	_current++;
	deltaX = 0.0f;
	deltaY = 0.0f;
	codepoints.clear();
	pressedKeys.clear();
	glfwPollEvents();

	for (auto& entry : bindings) {
		auto& binding = entry.second;
		binding.justChange = false;

		bool newstate = false;
		switch (binding.type) {
			case inputtype::keyboard: newstate = pressed(binding.code); break;
			case inputtype::button: newstate = clicked(binding.code); break;
		}

		if (newstate) {
			if (!binding.state) {
				binding.state = true;
				binding.justChange = true;
			}
		} else {
			if (binding.state) {
				binding.state = false;
				binding.justChange = true;
			}
		}
	}
}

void Events::bind(std::string name, inputtype type, int code) {
	bindings[name] = {type, code, false, false};
}

bool Events::active(std::string name) {
	const auto& found = bindings.find(name);
	if (found == bindings.end()) {
		return false;
	}
	return found->second.active();
}

bool Events::jactive(std::string name) {
	const auto& found = bindings.find(name);
	if (found == bindings.end()) {
		return false;
	}
	return found->second.jactive();
}
