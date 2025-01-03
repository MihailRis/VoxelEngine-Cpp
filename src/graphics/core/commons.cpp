#include "commons.hpp"

#include <map>

std::optional<CursorShape> CursorShape_from(std::string_view name) {
    static std::map<std::string_view, CursorShape> shapes = {
        {"arrow", CursorShape::ARROW},
        {"text", CursorShape::TEXT},
        {"crosshair", CursorShape::CROSSHAIR},
        {"pointer", CursorShape::POINTER},
        {"ew-resize", CursorShape::EW_RESIZE},
        {"ns-resize", CursorShape::NS_RESIZE},
        {"nwse-resize", CursorShape::NWSE_RESIZE},
        {"nesw-resize", CursorShape::NESW_RESIZE},
        {"all-resize", CursorShape::ALL_RESIZE},
        {"not-allowed", CursorShape::NOT_ALLOWED}
    };
    const auto& found = shapes.find(name);
    if (found == shapes.end()) {
        return std::nullopt;
    }
    return found->second;
}

std::string to_string(CursorShape shape) {
    static std::string names[] = {
        "arrow",
        "text",
        "crosshair",
        "pointer",
        "ew-resize",
        "ns-resize",
        "nwse-resize",
        "nesw-resize",
        "all-resize",
        "not-allowed"
    };
    return names[static_cast<int>(shape)];
}
