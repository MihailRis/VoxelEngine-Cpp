#include "platform.h"

#define SETTINGS_FILE "settings.json"


std::string platform::get_settings_file() {
	return SETTINGS_FILE;
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