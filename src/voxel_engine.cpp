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
#include "content/Content.h"

int main() {
	platform::configure_encoding();
	ContentBuilder contentBuilder;
	setup_definitions(&contentBuilder);

	std::unique_ptr<Content> content(contentBuilder.build());
	try {
	    EngineSettings settings;
		toml::Wrapper wrapper = create_wrapper(settings);

		std::filesystem::path settings_file = platform::get_settings_file();
		std::filesystem::path controls_file = platform::get_controls_file();
		if (std::filesystem::is_regular_file(settings_file)) {
			std::cout << "-- loading settings" << std::endl;
			std::string content = files::read_string(settings_file);
			toml::Reader reader(&wrapper, settings_file.string(), content);
			reader.read();
		}
		Engine engine(settings, content.get());
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
