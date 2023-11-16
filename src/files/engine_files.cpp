#include "engine_files.h"

#include <filesystem>
#include <sstream>
#include "../typedefs.h"

namespace filesystem = std::filesystem;
using std::string;
using filesystem::path;

path enginefs::get_screenshot_file(string ext) {
	path folder = SCREENSHOTS_FOLDER;
	if (!filesystem::is_directory(folder)) {
		filesystem::create_directory(folder);
	}

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%d-%m-%Y_%H-%M-%S";

	std::stringstream ss;
	ss << std::put_time(&tm, format);
	string datetimestr = ss.str();

	path filename = folder/("screenshot-"+datetimestr+"."+ext);
	uint index = 0;
	while (filesystem::exists(filename)) {
		filename = folder/("screenshot-"+datetimestr+"-"+std::to_string(index)+"."+ext);
		index++;
	}
	return filename;
}

path enginefs::get_worlds_folder() {
    return "worlds";
}