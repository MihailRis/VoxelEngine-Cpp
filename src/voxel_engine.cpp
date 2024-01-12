#include <iostream>
#include <cmath>
#include <stdint.h>
#include <memory>
#include <filesystem>
#include <stdexcept>

#include "definitions.h"
#include "engine.h"
#include "util/platform.h"
#include "coders/toml.h"
#include "files/files.h"
#include "files/settings_io.h"
#include "files/engine_paths.h"
#include "util/command_line.h"

#define SETTINGS_FILE "settings.toml"
#define CONTROLS_FILE "controls.json"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
	EnginePaths paths;
	if (!parse_cmdline(argc, argv, paths))
		return EXIT_SUCCESS;

	platform::configure_encoding();
    fs::path userfiles = paths.getUserfiles();
	try {
	    EngineSettings settings;
		std::unique_ptr<toml::Wrapper> wrapper (create_wrapper(settings));

		fs::path settings_file = userfiles/fs::path(SETTINGS_FILE);
		fs::path controls_file = userfiles/fs::path(CONTROLS_FILE);
		if (fs::is_regular_file(settings_file)) {
			std::cout << "-- loading settings" << std::endl;
			std::string text = files::read_string(settings_file);
			toml::Reader reader(wrapper.get(), settings_file.string(), text);
			reader.read();
		}
        setup_bindings();
		Engine engine(settings, &paths);
		if (fs::is_regular_file(controls_file)) {
			std::cout << "-- loading controls" << std::endl;
			std::string text = files::read_string(controls_file);
			load_controls(controls_file.string(), text);
		}
		engine.mainloop();
		
		std::cout << "-- saving settings" << std::endl;
		files::write_string(settings_file, wrapper->write());
		files::write_string(controls_file, write_controls());
	}
	catch (const initialize_error& err) {
		std::cerr << "could not to initialize engine" << std::endl;
		std::cerr << err.what() << std::endl;
	}
	return EXIT_SUCCESS;
}
