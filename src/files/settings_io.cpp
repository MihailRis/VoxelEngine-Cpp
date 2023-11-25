#include "settings_io.h"

#include <iostream>

#include "../window/Events.h"
#include "../window/input.h"

#include "../coders/json.h"

using std::string;

toml::Wrapper create_wrapper(EngineSettings& settings) {
	toml::Wrapper wrapper;
	toml::Section& display = wrapper.add("display");
	display.add("fullscreen", &settings.display.fullscreen);
	display.add("width", &settings.display.width);
	display.add("height", &settings.display.height);
	display.add("samples", &settings.display.samples);
	display.add("swap-interval", &settings.display.swapInterval);

	toml::Section& chunks = wrapper.add("chunks");
	chunks.add("load-distance", &settings.chunks.loadDistance);
	chunks.add("load-speed", &settings.chunks.loadSpeed);
	chunks.add("padding", &settings.chunks.padding);
	
	toml::Section& camera = wrapper.add("camera");
	camera.add("fov-effects", &settings.camera.fovEvents);
	camera.add("shaking", &settings.camera.shaking);

	toml::Section& graphics = wrapper.add("graphics");
	graphics.add("fog-curve", &settings.graphics.fogCurve);
	graphics.add("backlight", &settings.graphics.backlight);

	toml::Section& debug = wrapper.add("debug");
	debug.add("generator-test-mode", &settings.debug.generatorTestMode);
	debug.add("show-chunk-borders", &settings.debug.showChunkBorders);
	return wrapper;
}

string write_controls() {
	json::JObject* obj = new json::JObject();
	for (auto& entry : Events::bindings) {
		const auto& binding = entry.second;

		json::JObject* jentry = new json::JObject();
		switch (binding.type) {
			case inputtype::keyboard: jentry->put("type", "keyboard"); break;
			case inputtype::mouse: jentry->put("type", "mouse"); break;
			default: throw std::runtime_error("unsupported control type");
		}
		jentry->put("code", binding.code);
		obj->put(entry.first, jentry);
	}
	return json::stringify(obj, true, "  ");
}

void load_controls(string filename, string source) {
	json::JObject* obj = json::parse(filename, source);
	for (auto& entry : Events::bindings) {
		auto& binding = entry.second;

		json::JObject* jentry = obj->obj(entry.first);
		if (jentry == nullptr)
			continue;
		inputtype type;
		string typestr;
		jentry->str("type", typestr);

		if (typestr == "keyboard") {
			type = inputtype::keyboard;
		} else if (typestr == "mouse") {
			type = inputtype::mouse;
		} else {
			std::cerr << "unknown input type '" << typestr << "'" << std::endl;
			continue;
		}
		binding.type = type;
		jentry->num("code", binding.code);
	}
}