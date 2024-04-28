#ifndef FILES_SETTINGS_IO_HPP_
#define FILES_SETTINGS_IO_HPP_

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include "../settings.h"
#include "../data/dynamic.h"

struct Section {
    std::string name;
    std::vector<std::string> keys;
};

class SettingsHandler {
    std::unordered_map<std::string, Setting*> map;
    std::vector<Section> sections;
public:
    SettingsHandler(EngineSettings& settings);

    std::unique_ptr<dynamic::Value> getValue(const std::string& name) const;
    void setValue(const std::string& name, const dynamic::Value& value);
    std::string toString(const std::string& name) const;
    Setting* getSetting(const std::string& name) const;

    std::vector<Section>& getSections();
};

std::string write_controls();

void load_controls(std::string filename, std::string source);

#endif // FILES_SETTINGS_IO_HPP_
