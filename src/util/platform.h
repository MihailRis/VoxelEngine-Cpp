#ifndef UTIL_PLATFORM_H_
#define UTIL_PLATFORM_H_

#include <string>

namespace platform {
    extern void configure_encoding();
    extern std::string get_settings_file();
}

#endif // UTIL_PLATFORM_H_