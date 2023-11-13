#ifndef UTIL_PLATFORM_H_
#define UTIL_PLATFORM_H_

#include <string>

namespace platform {
    extern void configure_encoding();
    extern std::string get_settings_file();
    extern std::string get_screenshot_file(std::string ext);
}

#endif // UTIL_PLATFORM_H_