#include "platform.hpp"

#include "../typedefs.hpp"

#include <sstream>
#include <iomanip>
#include <time.h>
#include <iostream>

#ifdef _WIN32
#include <Windows.h>

#include "./stringutil.hpp"

void platform::configure_encoding() {
    // set utf-8 encoding to console output
    SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
}

std::string platform::detect_locale() {
    LCID lcid = GetThreadLocale();
    wchar_t preferredLocaleName[LOCALE_NAME_MAX_LENGTH];//locale name format: ll-CC
    if (LCIDToLocaleName(lcid, preferredLocaleName, LOCALE_NAME_MAX_LENGTH, 0) == 0) {
        std::cerr << "error in platform::detect_locale! LCIDToLocaleName failed." << std::endl;
    }
    //ll_CC format
    return util::wstr2str_utf8(preferredLocaleName).replace(2, 1, "_").substr(0, 5);
}

#else

void platform::configure_encoding(){
}

std::string platform::detect_locale() {
    std::string programLocaleName = setlocale(LC_ALL, nullptr);
    std::string preferredLocaleName = setlocale(LC_ALL, ""); //locale name format: ll_CC.encoding
    setlocale(LC_ALL, programLocaleName.c_str());

    return preferredLocaleName.substr(0, 5);
}

#endif
