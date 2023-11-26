#include "command_line.h"

#include <filesystem>

namespace fs = std::filesystem;

using std::filesystem::path;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

bool parse_cmdline(int argc, char** argv, EnginePaths& paths) {
	ArgsReader reader(argc, argv);
	reader.skip();
	while (reader.hasNext()) {
		string token = reader.next();
		if (reader.isKeywordArg()) {
			if (token == "--res") {
				token = reader.next();
				if (!fs::is_directory(path(token))) {
					throw std::runtime_error(token+" is not a directory");
				}
				paths.setResources(path(token));
				cout << "resources folder: " << token << std::endl;
			} else if (token == "--dir") {
				token = reader.next();
				if (!fs::is_directory(path(token))) {
					fs::create_directories(path(token));
				}
				paths.setUserfiles(path(token));
				cout << "userfiles folder: " << token << endl;
			} else if (token == "--help" || token == "-h") {
				cout << "VoxelEngine command-line arguments:" << endl;
				cout << " --res [path] - set resources directory" << endl;
				cout << " --dir [path] - set userfiles directory" << endl;
				return false;
			} else {
				cerr << "unknown argument " << token << endl;
			}
		} else {
			cerr << "unexpected token" << endl;
		}
	}
	return true;
}