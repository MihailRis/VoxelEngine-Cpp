#ifndef CODERS_TOML_H_
#define CODERS_TOML_H_

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

#endif // CODERS_TOML_H_
