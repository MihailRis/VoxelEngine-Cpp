#pragma once

#include "presets/NotePreset.hpp"

/// @brief 3D text instance
class TextNote {
    std::wstring text;
    NotePreset preset;
    glm::vec3 position;
public:
    TextNote(std::wstring text, NotePreset preset, glm::vec3 position);

    const std::wstring& getText() const;

    const NotePreset& getPreset() const;

    const glm::vec3& getPosition() const;
};
