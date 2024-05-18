#include "settings_io.hpp"

#include "../window/Events.hpp"
#include "../window/input.hpp"
#include "../coders/toml.hpp"
#include "../coders/json.hpp"
#include "../debug/Logger.hpp"
#include "../settings.hpp"

#include <memory>

static debug::Logger logger("settings_io");

struct SectionsBuilder {
    std::unordered_map<std::string, Setting*>& map;
    std::vector<Section>& sections;

    SectionsBuilder(
        std::unordered_map<std::string, Setting*>& map,
        std::vector<Section>& sections
    ) : map(map), sections(sections) {
    }

    void section(std::string name) {
        sections.push_back(Section {name, {}});
    }

    void add(std::string name, Setting* setting, bool writeable=true) {
        Section& section = sections.at(sections.size()-1);
        map[section.name+"."+name] = setting;
        section.keys.push_back(name);
    }
};

SettingsHandler::SettingsHandler(EngineSettings& settings) {
    SectionsBuilder builder(map, sections);

    builder.section("audio");
    builder.add("enabled", &settings.audio.enabled, false);
    builder.add("volume-master", &settings.audio.volumeMaster);
    builder.add("volume-regular", &settings.audio.volumeRegular);
    builder.add("volume-ui", &settings.audio.volumeUI);
    builder.add("volume-ambient", &settings.audio.volumeAmbient);
    builder.add("volume-music", &settings.audio.volumeMusic);

    builder.section("display");
    builder.add("width", &settings.display.width);
    builder.add("height", &settings.display.height);
    builder.add("samples", &settings.display.samples);
    builder.add("vsync", &settings.display.vsync);
    builder.add("fullscreen", &settings.display.fullscreen);

    builder.section("camera");
    builder.add("sensitivity", &settings.camera.sensitivity);
    builder.add("fov", &settings.camera.fov);
    builder.add("fov-effects", &settings.camera.fovEffects);
    builder.add("shaking", &settings.camera.shaking);

    builder.section("chunks");
    builder.add("load-distance", &settings.chunks.loadDistance);
    builder.add("load-speed", &settings.chunks.loadSpeed);
    builder.add("padding", &settings.chunks.padding);

    builder.section("graphics");
    builder.add("fog-curve", &settings.graphics.fogCurve);
    builder.add("backlight", &settings.graphics.backlight);
    builder.add("gamma", &settings.graphics.gamma);
    builder.add("frustum-culling", &settings.graphics.frustumCulling);
    builder.add("skybox-resolution", &settings.graphics.skyboxResolution);

    builder.section("ui");
    builder.add("language", &settings.ui.language);
    builder.add("world-preview-size", &settings.ui.worldPreviewSize);

    builder.section("debug");
    builder.add("generator-test-mode", &settings.debug.generatorTestMode);
    builder.add("do-write-lights", &settings.debug.doWriteLights);
}

dynamic::Value SettingsHandler::getValue(const std::string& name) const {
    auto found = map.find(name);
    if (found == map.end()) {
        throw std::runtime_error("setting '"+name+"' does not exist");
    }
    auto setting = found->second;
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        return static_cast<number_t>(number->get());
    } else if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
        return static_cast<integer_t>(integer->get());
    } else if (auto flag = dynamic_cast<FlagSetting*>(setting)) {
        return flag->get();
    } else if (auto string = dynamic_cast<StringSetting*>(setting)) {
        return string->get();
    } else {
        throw std::runtime_error("type is not implemented for '"+name+"'");
    }
}

std::string SettingsHandler::toString(const std::string& name) const {
    auto found = map.find(name);
    if (found == map.end()) {
        throw std::runtime_error("setting '"+name+"' does not exist");
    }
    auto setting = found->second;
    return setting->toString();
}

Setting* SettingsHandler::getSetting(const std::string& name) const {
    auto found = map.find(name);
    if (found == map.end()) {
        throw std::runtime_error("setting '"+name+"' does not exist");
    }
    return found->second;
}

bool SettingsHandler::has(const std::string& name) const {
    return map.find(name) != map.end();
}

template<class T>
static void set_numeric_value(T* setting, const dynamic::Value& value) {
    if (auto num = std::get_if<integer_t>(&value)) {
        setting->set(*num);
    } else if (auto num = std::get_if<number_t>(&value)) {
        setting->set(*num);
    } else if (auto flag = std::get_if<bool>(&value)) {
        setting->set(*flag);
    } else {
        throw std::runtime_error("type error, numeric value expected");
    }
}

void SettingsHandler::setValue(const std::string& name, const dynamic::Value& value) {
    auto found = map.find(name);
    if (found == map.end()) {
        throw std::runtime_error("setting '"+name+"' does not exist");
    }
    auto setting = found->second;
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        set_numeric_value(number, value);
    } else if (auto integer = dynamic_cast<IntegerSetting*>(setting)) {
        set_numeric_value(integer, value);
    } else if (auto flag = dynamic_cast<FlagSetting*>(setting)) {
        set_numeric_value(flag, value);
    } else if (auto string = dynamic_cast<StringSetting*>(setting)) {
        if (auto num = std::get_if<integer_t>(&value)) {
            string->set(std::to_string(*num));
        } else if (auto num = std::get_if<number_t>(&value)) {
            string->set(std::to_string(*num));
        } else if (auto flag = std::get_if<bool>(&value)) {
            string->set(*flag ? "true" : "false");
        } else if (auto str = std::get_if<std::string>(&value)) {
            string->set(*str);
        } else {
            throw std::runtime_error("not implemented for type");
        }
    } else {
        throw std::runtime_error("type is not implement - setting '"+name+"'");
    }
}

std::vector<Section>& SettingsHandler::getSections() {
    return sections;
}
