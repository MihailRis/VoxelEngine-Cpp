#include "settings_io.h"

#include <memory>
#include <iostream>

#include "../window/Events.h"
#include "../window/input.h"

#include "../coders/toml.h"
#include "../coders/json.h"

SettingsHandler::SettingsHandler(EngineSettings& settings) {
    map.emplace("audio.volume-master", &settings.audio.volumeMaster);
    map.emplace("audio.volume-regular", &settings.audio.volumeRegular);
    map.emplace("audio.volume-ui", &settings.audio.volumeUI);
    map.emplace("audio.volume-ambient", &settings.audio.volumeAmbient);
    map.emplace("audio.volume-music", &settings.audio.volumeMusic);

    map.emplace("camera.sensitivity", &settings.camera.sensitivity);
}

dynamic::Value SettingsHandler::getValue(const std::string& name) const {
    auto found = map.find(name);
    if (found == map.end()) {
        throw std::runtime_error("setting '"+name+"' does not exist");
    }
    auto setting = found->second;
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        return dynamic::Value::of((number_t)number->get());
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

void SettingsHandler::setValue(const std::string& name, dynamic::Value value) {
    auto found = map.find(name);
    if (found == map.end()) {
        throw std::runtime_error("setting '"+name+"' does not exist");
    }
    auto setting = found->second;
    if (auto number = dynamic_cast<NumberSetting*>(setting)) {
        switch (value.type) {
            case dynamic::valtype::integer:
                number->set(std::get<integer_t>(value.value));
                break;
            case dynamic::valtype::number:
                number->set(std::get<number_t>(value.value));
                break;
            default:
                throw std::runtime_error("type error, numeric value expected");
        }
    } else {
        throw std::runtime_error("type is not implement - setting '"+name+"'");
    }
}

toml::Wrapper* create_wrapper(EngineSettings& settings) {
    auto wrapper = std::make_unique<toml::Wrapper>();

    toml::Section& audio = wrapper->add("audio");
    audio.add("enabled", &settings.audio.enabled);
    audio.add("volume-master", &*settings.audio.volumeMaster);
    audio.add("volume-regular", &*settings.audio.volumeRegular);
    audio.add("volume-ui", &*settings.audio.volumeUI);
    audio.add("volume-ambient", &*settings.audio.volumeAmbient);
    audio.add("volume-music", &*settings.audio.volumeMusic);

    toml::Section& display = wrapper->add("display");
    display.add("fullscreen", &settings.display.fullscreen);
    display.add("width", &settings.display.width);
    display.add("height", &settings.display.height);
    display.add("samples", &settings.display.samples);
    display.add("swap-interval", &settings.display.swapInterval);

    toml::Section& chunks = wrapper->add("chunks");
    chunks.add("load-distance", &settings.chunks.loadDistance);
    chunks.add("load-speed", &settings.chunks.loadSpeed);
    chunks.add("padding", &settings.chunks.padding);
    
    toml::Section& camera = wrapper->add("camera");
    camera.add("fov-effects", &settings.camera.fovEvents);
    camera.add("fov", &settings.camera.fov);
    camera.add("shaking", &settings.camera.shaking);
    camera.add("sensitivity", &*settings.camera.sensitivity);

    toml::Section& graphics = wrapper->add("graphics");
    graphics.add("gamma", &settings.graphics.gamma);
    graphics.add("fog-curve", &settings.graphics.fogCurve);
    graphics.add("backlight", &settings.graphics.backlight);
    graphics.add("frustum-culling", &settings.graphics.frustumCulling);
    graphics.add("skybox-resolution", &settings.graphics.skyboxResolution);

    toml::Section& debug = wrapper->add("debug");
    debug.add("generator-test-mode", &settings.debug.generatorTestMode);
    debug.add("show-chunk-borders", &settings.debug.showChunkBorders);
    debug.add("do-write-lights", &settings.debug.doWriteLights);

    toml::Section& ui = wrapper->add("ui");
    ui.add("language", &settings.ui.language);
    return wrapper.release();
}

std::string write_controls() {
    dynamic::Map obj;
    for (auto& entry : Events::bindings) {
        const auto& binding = entry.second;

        auto& jentry = obj.putMap(entry.first);
        switch (binding.type) {
            case inputtype::keyboard: jentry.put("type", "keyboard"); break;
            case inputtype::mouse: jentry.put("type", "mouse"); break;
            default: throw std::runtime_error("unsupported control type");
        }
        jentry.put("code", binding.code);
    }
    return json::stringify(&obj, true, "  ");
}

void load_controls(std::string filename, std::string source) {
    auto obj = json::parse(filename, source);
    for (auto& entry : Events::bindings) {
        auto& binding = entry.second;

        auto jentry = obj->map(entry.first);
        if (jentry == nullptr)
            continue;
        inputtype type;
        std::string typestr;
        jentry->str("type", typestr);

        if (typestr == "keyboard") {
            type = inputtype::keyboard;
        } else if (typestr == "mouse") {
            type = inputtype::mouse;
        } else {
            std::cerr << "unknown input type '" << typestr << "'" << std::endl;
            continue;
        }
        binding.type = type;
        jentry->num("code", binding.code);
    }
}
