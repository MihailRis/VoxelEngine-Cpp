#include "engine_paths.h"

#include <filesystem>
#include <sstream>
#include "../typedefs.h"

#define SCREENSHOTS_FOLDER "screenshots"

namespace fs = std::filesystem;
using std::string;
using fs::path;

path EnginePaths::getUserfiles() const {
	return userfiles;
}

path EnginePaths::getResources() const {
	return resources;
}

path EnginePaths::getScreenshotFile(string ext) {
	path folder = SCREENSHOTS_FOLDER;
	if (!fs::is_directory(folder)) {
		fs::create_directory(folder);
	}

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%Y-%m-%d_%H-%M-%S";
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

path EnginePaths::getWorldsFolder() {
    return path("worlds");
}

bool EnginePaths::isWorldNameUsed(string name) {
	return fs::exists(EnginePaths::getWorldsFolder()/fs::u8path(name));
}

void EnginePaths::setUserfiles(path folder) {
	this->userfiles = folder;
}

void EnginePaths::setResources(path folder) {
	this->resources = folder;
}
