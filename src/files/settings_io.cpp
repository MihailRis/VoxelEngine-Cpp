#include "settings_io.h"

#include <memory>
#include <iostream>

#include "../window/Events.h"
#include "../window/input.h"

#include "../coders/toml.h"
#include "../coders/json.h"

#include "../data/dynamic.h"

toml::Wrapper* create_wrapper(EngineSettings& settings) {
	std::unique_ptr<toml::Wrapper> wrapper (new toml::Wrapper());
	toml::Section& display = wrapper->add("display");
	display.add("fullscreen", &settings.display.fullscreen);
	display.add("width", &settings.display.width);
	display.add("height", &settings.display.height);
	display.add("samples", &settings.display.samples);
	display.add("swap-interval", &settings.display.swapInterval);

	toml::Section& chunks = wrapper->add("chunks");
	chunks.add("load-distance", &settings.chunks.loadDistance);
	chunks.add("load-speed", &settings.chunks.loadSpeed);
	chunks.add("padding", &settings.chunks.padding);
	
	toml::Section& camera = wrapper->add("camera");
	camera.add("fov-effects", &settings.camera.fovEvents);
	camera.add("fov", &settings.camera.fov);
	camera.add("shaking", &settings.camera.shaking);
	camera.add("sensitivity", &settings.camera.sensitivity);

	toml::Section& graphics = wrapper->add("graphics");
	graphics.add("fog-curve", &settings.graphics.fogCurve);
	graphics.add("backlight", &settings.graphics.backlight);
	graphics.add("frustum-culling", &settings.graphics.frustumCulling);
	graphics.add("skybox-resolution", &settings.graphics.skyboxResolution);

	toml::Section& debug = wrapper->add("debug");
	debug.add("generator-test-mode", &settings.debug.generatorTestMode);
	debug.add("show-chunk-borders", &settings.debug.showChunkBorders);
	debug.add("do-write-lights", &settings.debug.doWriteLights);

    toml::Section& ui = wrapper->add("ui");
    ui.add("language", &settings.ui.language);
	return wrapper.release();
}

std::string write_controls() {
	dynamic::Map obj;
	for (auto& entry : Events::bindings) {
		const auto& binding = entry.second;

        auto& jentry = obj.putMap(entry.first);
		switch (binding.type) {
			case inputtype::keyboard: jentry.put("type", "keyboard"); break;
			case inputtype::mouse: jentry.put("type", "mouse"); break;
			default: throw std::runtime_error("unsupported control type");
		}
		jentry.put("code", binding.code);
	}
	return json::stringify(&obj, true, "  ");
}

void load_controls(std::string filename, std::string source) {
    auto obj = json::parse(filename, source);
	for (auto& entry : Events::bindings) {
		auto& binding = entry.second;

		auto jentry = obj->map(entry.first);
		if (jentry == nullptr)
			continue;
		inputtype type;
		std::string typestr;
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
