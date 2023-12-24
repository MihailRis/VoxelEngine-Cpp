#include "platform.h"

#include <sstream>
#include <iomanip>
#include <time.h>

#include "../typedefs.h"

std::string platform::detect_locale() {
    // TODO: implement
    std::string name = setlocale(LC_ALL, nullptr);
    if (name.find("ru_RU") != std::string::npos) {
        return "ru_RU";
    }
    return "en_US";
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
