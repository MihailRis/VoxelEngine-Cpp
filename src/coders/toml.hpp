#ifndef CODERS_TOML_HPP_
#define CODERS_TOML_HPP_

#include "commons.hpp"

#include <string>

class SettingsHandler;

namespace toml {
    std::string stringify(SettingsHandler& handler);

    void parse(
        SettingsHandler& handler, 
        const std::string& file, 
        const std::string& source
    );
}

#endif // CODERS_TOML_HPP_
