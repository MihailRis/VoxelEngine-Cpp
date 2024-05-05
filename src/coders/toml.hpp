#ifndef CODERS_TOML_HPP_
#define CODERS_TOML_HPP_

#include <string>
#include <vector>
#include <unordered_map>

#include "commons.h"

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
