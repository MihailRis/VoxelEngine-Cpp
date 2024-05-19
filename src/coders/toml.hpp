#ifndef CODERS_TOML_HPP_
#define CODERS_TOML_HPP_

#include "../data/dynamic.hpp"
#include "commons.hpp"

#include <string>

class SettingsHandler;

namespace toml {
    std::string stringify(SettingsHandler& handler);
    std::string stringify(dynamic::Map& root);
    dynamic::Map_sptr parse(std::string_view file, std::string_view source);

    void parse(
        SettingsHandler& handler, 
        std::string_view file, 
        std::string_view source
    );
}

#endif // CODERS_TOML_HPP_
