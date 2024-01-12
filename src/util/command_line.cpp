#include "command_line.h"

#include <filesystem>

namespace fs = std::filesystem;

bool parse_cmdline(int argc, char** argv, EnginePaths& paths) {
	ArgsReader reader(argc, argv);
	reader.skip();
	while (reader.hasNext()) {
		std::string token = reader.next();
		if (reader.isKeywordArg()) {
			if (token == "--res") {
				token = reader.next();
				if (!fs::is_directory(fs::path(token))) {
					throw std::runtime_error(token+" is not a directory");
				}
				paths.setResources(fs::path(token));
				std::cout << "resources folder: " << token << std::endl;
			} else if (token == "--dir") {
				token = reader.next();
				if (!fs::is_directory(fs::path(token))) {
					fs::create_directories(fs::path(token));
				}
				paths.setUserfiles(fs::path(token));
				std::cout << "userfiles folder: " << token << std::endl;
			} else if (token == "--help" || token == "-h") {
				std::cout << "VoxelEngine command-line arguments:" << std::endl;
				std::cout << " --res [path] - set resources directory" << std::endl;
				std::cout << " --dir [path] - set userfiles directory" << std::endl;
				return false;
			} else {
				std::cerr << "unknown argument " << token << std::endl;
			}
		} else {
			std::cerr << "unexpected token" << std::endl;
		}
	}
	return true;
}