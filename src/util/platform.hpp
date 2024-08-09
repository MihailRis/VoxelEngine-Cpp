#pragma once

#include <string>

namespace platform {
    void configure_encoding();
    // @return environment locale in ISO format ll_CC
    std::string detect_locale();
}
