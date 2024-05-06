#ifndef FILES_SETTINGS_IO_HPP_
#define FILES_SETTINGS_IO_HPP_

#include "../data/dynamic.hpp"

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

class Setting;
struct EngineSettings;

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
    bool has(const std::string& name) const;

    std::vector<Section>& getSections();
};

#endif // FILES_SETTINGS_IO_HPP_
