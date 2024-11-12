#pragma once

#include <glm/vec3.hpp>
#include <optional>

#include "interfaces/Serializable.hpp"

enum class NoteDisplayMode {
    STATIC_BILLBOARD,
    Y_FREE_BILLBOARD,
    XY_FREE_BILLBOARD,
    PROJECTED
};

std::string to_string(NoteDisplayMode mode);
std::optional<NoteDisplayMode> NoteDisplayMode_from(std::string_view s);

struct NotePreset : public Serializable {
    NoteDisplayMode displayMode = NoteDisplayMode::STATIC_BILLBOARD;

    dv::value serialize() const override;
    void deserialize(const dv::value& src) override;
};
