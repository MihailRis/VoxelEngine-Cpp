#ifndef UTIL_PLATFORM_HPP_
#define UTIL_PLATFORM_HPP_

#include <string>

namespace platform {
    void configure_encoding();
    // @return environment locale in ISO format ll_CC
    std::string detect_locale();
}

#endif  // UTIL_PLATFORM_HPP_
