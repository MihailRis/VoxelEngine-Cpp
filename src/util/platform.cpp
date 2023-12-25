#include "platform.h"

#include <sstream>
#include <iomanip>
#include <time.h>
#include <iostream>

#include "../typedefs.h"

namespace platform {
    const std::string SETTINGS_FILE = "settings.toml";
    const std::string CONTROLS_FILE = "controls.json";
    const std::string DEFAULT_LOCALE = "en_EN";
}

using std::filesystem::path;


path platform::get_settings_file() {
	return path(SETTINGS_FILE);
}

path platform::get_controls_file() {
	return path(CONTROLS_FILE);
}

/*System locale to engine locale mapping*/
std::string platform::get_locale_by_lang(std::string lang) {
    if (lang == "ru") {
        return "ru_RU";
    }
    return DEFAULT_LOCALE;
}

#ifdef WIN32
#include <Windows.h>

#include "../util/stringutil.h"

void platform::configure_encoding() {
	// set utf-8 encoding to console output
	SetConsoleOutputCP(CP_UTF8);
    setvbuf(stdout, nullptr, _IOFBF, 1000);
}

std::string platform::detect_locale() {
    LCID lcid = GetThreadLocale();
    wchar_t preferredLocaleName[LOCALE_NAME_MAX_LENGTH];
    if (LCIDToLocaleName(lcid, preferredLocaleName, LOCALE_NAME_MAX_LENGTH, 0) == 0) {
        std::cout << "error in platform::detect_locale! LCIDToLocaleName failed." << std::endl;
    }
    wchar_t parentLocaleName[LOCALE_NAME_MAX_LENGTH];
    if (GetLocaleInfoEx(preferredLocaleName, LOCALE_SPARENT, parentLocaleName, LOCALE_NAME_MAX_LENGTH) == 0){
        std::cout << "error in platform::detect_locale! GetLocaleInfoEx failed." << std::endl;
    }
    std::wcout << "detected environment language locale: " << parentLocaleName << std::endl;

    std::string preferredLang = util::wstr2str_utf8(parentLocaleName);
    return get_locale_by_lang(preferredLang);
}

#else

void platform::configure_encoding(){
}

std::string platform::detect_locale() {
    std::string programLocaleName = setlocale(LC_ALL, nullptr);
    std::string preferredLocaleName = setlocale(LC_ALL, "");
    std::cout << "detected environment locale: " << preferredLocaleName << std::endl;
    setlocale(LC_ALL, programLocaleName.c_str());

    std::string preferredLang = preferredLocaleName.substr(0, 2);
    return get_locale_by_lang(preferredLang);
}

#endif