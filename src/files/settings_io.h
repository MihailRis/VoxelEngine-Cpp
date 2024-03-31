#ifndef FILES_SETTINGS_IO_H_
#define FILES_SETTINGS_IO_H_

#include <string>
#include <unordered_map>
#include "../settings.h"
#include "../data/dynamic.h"

namespace toml {
    class Wrapper;
}

class SettingsHandler {
    std::unordered_map<std::string, Setting*> map;
public:
    SettingsHandler(EngineSettings& settings);

    dynamic::Value getValue(const std::string& name) const;
    void setValue(const std::string& name, dynamic::Value value);
};

extern std::string write_controls();
extern toml::Wrapper* create_wrapper(EngineSettings& settings);
extern void load_controls(std::string filename, std::string source);

#endif // FILES_SETTINGS_IO_H_
