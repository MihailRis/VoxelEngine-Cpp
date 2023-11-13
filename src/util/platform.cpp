#include "platform.h"

#include <sstream>
#include <filesystem>
#include <iomanip>
#include <time.h>

#include "../typedefs.h"

#define SETTINGS_FILE "settings.json"
#define SCREENSHOTS_FOLDER "screenshots"

using std::string;


string platform::get_settings_file() {
	return SETTINGS_FILE;
}

string platform::get_screenshot_file(string ext) {
	std::string folder = SCREENSHOTS_FOLDER;
	if (!std::filesystem::is_directory(folder)) {
		std::filesystem::create_directory(folder);
	}

	auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

	const char* format = "%d-%m-%Y_%H-%M-%S";

	std::stringstream ss;
	ss << std::put_time(&tm, format);
	string datetimestr = ss.str();

	string filename = folder+"/screenshot-"+datetimestr+"."+ext;
	uint index = 0;
	while (std::filesystem::exists(filename)) {
		filename = folder+"/screenshot-"+datetimestr+"-"+std::to_string(index)+"."+ext;
		index++;
	}
	return filename;
}

#ifdef WIN32
#include <Windows.h>

void platform::configure_encoding() {
	// set utf-8 encoding to console output
	SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
}
#else
void platform::configure_encoding(){
}

#endif