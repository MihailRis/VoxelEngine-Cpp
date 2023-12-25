#ifndef UTIL_PLATFORM_H_
#define UTIL_PLATFORM_H_

#include <string>
#include <filesystem>

namespace platform {
    extern void configure_encoding();
    extern std::filesystem::path get_settings_file();
    extern std::filesystem::path get_controls_file();
    extern std::string detect_locale();
    extern std::string get_locale_by_lang(std::string lang);
}

#endif // UTIL_PLATFORM_H_