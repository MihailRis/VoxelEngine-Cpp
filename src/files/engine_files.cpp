#include "engine_files.h"

#include <filesystem>
#include <sstream>
#include "../typedefs.h"

namespace fs = std::filesystem;
using std::string;
using fs::path;

path enginefs::get_screenshot_file(string ext) {
	path folder = SCREENSHOTS_FOLDER;
	if (!fs::is_directory(folder)) {
		fs::create_directory(folder);
	}

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%d-%m-%Y_%H-%M-%S";

	std::stringstream ss;
	ss << std::put_time(&tm, format);
	string datetimestr = ss.str();

	path filename = folder/path("screenshot-"+datetimestr+"."+ext);
	uint index = 0;
	while (fs::exists(filename)) {
		filename = folder/path("screenshot-"+datetimestr+"-"+std::to_string(index)+"."+ext);
		index++;
	}
	return filename;
}

path enginefs::get_worlds_folder() {
    return path("worlds");
}

bool enginefs::is_world_name_used(std::string name) {
	return fs::exists(enginefs::get_worlds_folder()/fs::u8path(name));
}