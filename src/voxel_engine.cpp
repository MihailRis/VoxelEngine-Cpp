#include <iostream>
#include <cmath>
#include <stdint.h>

#include <memory>
#include <filesystem>

#include "definitions.h"

#include "util/platform.h"
#include "engine.h"

#include "coders/toml.h"
#include "files/files.h"

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

int main() {
	platform::configure_encoding();
	setup_definitions();
	try {
	    EngineSettings settings;
		toml::Wrapper wrapper = create_wrapper(settings);

		std::string settings_file = platform::get_settings_file();
		if (std::filesystem::is_regular_file(settings_file)) {
			std::cout << "-- loading settings" << std::endl;
			std::string content = files::read_string(settings_file);
			toml::Reader reader(&wrapper, settings_file, content);
			reader.read();
		} else {
			std::cout << "-- creating settings file " << settings_file << std::endl;
			files::write_string(settings_file, wrapper.write());
		}
		Engine engine(settings);
		engine.mainloop();
		
		std::cout << "-- saving settings" << std::endl;
		files::write_string(settings_file, wrapper.write());
	}
	catch (const initialize_error& err) {
		std::cerr << "could not to initialize engine" << std::endl;
		std::cerr << err.what() << std::endl;
	}
	return 0;
}
