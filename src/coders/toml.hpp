#pragma once

#include <string>

#include "data/dv.hpp"

class SettingsHandler;

namespace toml {
    std::string stringify(SettingsHandler& handler);
    std::string stringify(const dv::value& root, const std::string& name = "");

    dv::value parse(std::string_view file, std::string_view source);

    void parse(
        SettingsHandler& handler, std::string_view file, std::string_view source
    );
}
