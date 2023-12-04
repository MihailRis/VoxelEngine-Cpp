#include <iostream>
#include "Events.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string.h>

const short KEYS_BUFFER_SIZE = 1032;
const short _MOUSE_KEYS_OFFSET = 1024;

bool* Events::_keys;
uint* Events::_frames;
uint Events::_current = 0;
float Events::deltaX = 0.0f;
float Events::deltaY = 0.0f;
float Events::x = 0.0f;
float Events::y = 0.0f;
int Events::scroll = 0;
bool Events::_cursor_locked = false;
bool Events::_cursor_started = false;
std::vector<uint> Events::codepoints;
std::vector<int> Events::pressedKeys;
std::unordered_map<std::string, Binding> Events::bindings;

int Events::initialize(){
	_keys = new bool[KEYS_BUFFER_SIZE];
	_frames = new uint[KEYS_BUFFER_SIZE];

	memset(_keys, false, KEYS_BUFFER_SIZE*sizeof(bool));
	memset(_frames, 0, KEYS_BUFFER_SIZE*sizeof(uint));

	return 0;
}

void Events::finalize(){
	delete[] _keys;
	delete[] _frames;
}

// Returns bool repr. of key state
bool Events::pressed(int keycode){
	if (keycode < 0 || keycode >= KEYS_BUFFER_SIZE){
		// VERY bad behaviour and it happens constantly! (so console-printing is not a good idea)
		return false;
	}
	return _keys[keycode];
}

// Returns bool repr. of key state
bool Events::jpressed(int keycode){ 
	return Events::pressed(keycode) && _frames[keycode] == _current;
}

// Returns bool repr. of mouse key state
bool Events::clicked(int button){
	return Events::pressed(_MOUSE_KEYS_OFFSET + button);
}

// Returns bool repr. of mouse key state
bool Events::jclicked(int button){
	return Events::jpressed(_MOUSE_KEYS_OFFSET + button);
}

void Events::toggleCursor(){
	_cursor_locked = !_cursor_locked;
	Window::setCursorMode(_cursor_locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

void Events::pollEvents(){
	_current++;
	deltaX = 0.0f;
	deltaY = 0.0f;
	scroll = 0;
	codepoints.clear();
	pressedKeys.clear();
	glfwPollEvents();

	for (auto& entry : bindings) {
		auto& binding = entry.second;
		binding.justChange = false;

		bool newstate = false;
		switch (binding.type) {
			case inputtype::keyboard: newstate = pressed(binding.code); break;
			case inputtype::mouse: newstate = clicked(binding.code); break;
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
