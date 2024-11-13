#include "NotePreset.hpp"

#include <map>
#include <vector>

#include "data/dv_util.hpp"

std::string to_string(NoteDisplayMode mode) {
    static std::vector<std::string> names = {
        "static_billboard",
        "y_free_billboard",
        "xy_free_billboard",
        "projected"
    };
    return names.at(static_cast<int>(mode));
}

std::optional<NoteDisplayMode> NoteDisplayMode_from(std::string_view s) {
    static std::map<std::string_view, NoteDisplayMode, std::less<>> map {
        {"static_billboard", NoteDisplayMode::STATIC_BILLBOARD},
        {"y_free_billboard", NoteDisplayMode::Y_FREE_BILLBOARD},
        {"xy_free_billboard", NoteDisplayMode::XY_FREE_BILLBOARD},
        {"projected", NoteDisplayMode::PROJECTED}
    };
    const auto& found = map.find(s);
    if (found == map.end()) {
        return std::nullopt;
    }
    return found->second;
}

dv::value NotePreset::serialize() const {
    return dv::object({
        {"display", to_string(displayMode)},
        {"color", dv::to_value(color)},
        {"scale", scale},
        {"render_distance", renderDistance}
    });
}

void NotePreset::deserialize(const dv::value& src) {
    if (src.has("display")) {
        displayMode = NoteDisplayMode_from(src["display"].asString()).value();
    }
    if (src.has("color")) {
        dv::get_vec(src["color"], color);
    }
    src.at("scale").get(scale);
    src.at("render_distance").get(renderDistance);
}
