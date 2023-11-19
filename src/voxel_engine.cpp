#include <iostream>
#include <cmath>
#include <stdint.h>
#include <memory>
#include <filesystem>
#include <stdexcept>

#include "definitions.h"

#include "util/platform.h"
#include "engine.h"

#include "coders/toml.h"
#include "coders/json.h"
#include "files/files.h"

#include "window/Events.h"

toml::Wrapper create_wrapper(EngineSettings& settings) {
	toml::Wrapper wrapper;
	toml::Section& display = wrapper.add("display");
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

	toml::Section& debug = wrapper.add("debug");
	debug.add("generator-test-mode", &settings.debug.generatorTestMode);
	return wrapper;
}

std::string write_controls() {
	json::JObject* obj = new json::JObject();
	for (auto& entry : Events::bindings) {
		const auto& binding = entry.second;

		json::JObject* jentry = new json::JObject();
		switch (binding.type) {
			case inputtype::keyboard: jentry->put("type", "keyboard"); break;
			case inputtype::button: jentry->put("type", "button"); break;
			default: throw std::runtime_error("unsupported control type");
		}
		jentry->put("code", binding.code);
		obj->put(entry.first, jentry);
	}
	return json::stringify(obj, true, "  ");
}

void load_controls(std::string filename, std::string source) {
	json::JObject* obj = json::parse(filename, source);
	for (auto& entry : Events::bindings) {
		auto& binding = entry.second;

		json::JObject* jentry = obj->obj(entry.first);
		if (jentry == nullptr)
			continue;
		inputtype type;
		std::string typestr;
		jentry->str("type", typestr);

		if (typestr == "keyboard") {
			type = inputtype::keyboard;
		} else if (typestr == "button") {
			type = inputtype::button;
		} else {
			std::cerr << "unknown input type '" << typestr << "'" << std::endl;
			continue;
		}
		binding.type = type;
		jentry->num("code", binding.code);
	}
}

int main() {
	platform::configure_encoding();
	setup_definitions();
	try {
	    EngineSettings settings;
		toml::Wrapper wrapper = create_wrapper(settings);

		std::filesystem::path settings_file = platform::get_settings_file();
		std::filesystem::path controls_file = platform::get_controls_file();
		if (std::filesystem::is_regular_file(settings_file)) {
			std::cout << "-- loading settings" << std::endl;
			std::string content = files::read_string(settings_file);
			toml::Reader reader(&wrapper, settings_file, content);
			reader.read();
		}
		Engine engine(settings);
		setup_bindings();
		if (std::filesystem::is_regular_file(controls_file)) {
			std::cout << "-- loading controls" << std::endl;
			std::string content = files::read_string(controls_file);
			load_controls(controls_file.string(), content);
		}
		engine.mainloop();
		
		std::cout << "-- saving settings" << std::endl;
		files::write_string(settings_file, wrapper.write());
		files::write_string(controls_file, write_controls());
	}
	catch (const initialize_error& err) {
		std::cerr << "could not to initialize engine" << std::endl;
		std::cerr << err.what() << std::endl;
	}
	return 0;
}
