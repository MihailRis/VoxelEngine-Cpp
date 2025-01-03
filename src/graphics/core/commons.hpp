#pragma once

#include <string>
#include <optional>

enum class DrawPrimitive {
    point = 0,
    line,
    triangle,
};

enum class BlendMode {
    /// @brief Normal blending mode.
    normal, 
    /// @brief Additive blending mode.
    addition, 
    /// @brief Subtractive blending mode.
    inversion
};

/// @brief Standard GLFW 3.4 cursor shapes (same order and count as in GLFW).
/// It also works in GLFW 3.3 (unsupported shapes will be replaced with the arrow).
enum class CursorShape {
    /// @brief Regular arrow
    ARROW,
    /// @brief Text input I-beam
    TEXT,
    /// @brief Crosshair 
    CROSSHAIR,
    /// @brief Pointing hand
    POINTER,
    /// @brief Horizontal resize arrow
    EW_RESIZE,
    /// @brief Vertical resize arrow
    NS_RESIZE,
    
    // GLFW 3.4+ cursor shapes
    
    /// @brief Diagonal resize arrow (top-left to bottom-right)
    NWSE_RESIZE,
    /// @brief Diagonal resize arrow (top-right to bottom-left)
    NESW_RESIZE,
    /// @brief All-direction resize arrow
    ALL_RESIZE,
    /// @brief Operation not allowed
    NOT_ALLOWED,
    
    LAST=NOT_ALLOWED
};

std::optional<CursorShape> CursorShape_from(std::string_view name);
std::string to_string(CursorShape shape);

class Flushable {
public:
    virtual ~Flushable() = default;

    virtual void flush() = 0;
};
