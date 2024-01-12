#ifndef UTIL_PLATFORM_H_
#define UTIL_PLATFORM_H_

#include <string>

namespace platform {
    extern void configure_encoding();
    // @return environment locale in ISO format ll_CC
    extern std::string detect_locale();
}

#endif // UTIL_PLATFORM_H_