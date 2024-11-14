#pragma once

#include "presets/NotePreset.hpp"

/// @brief 3D text instance
class TextNote {
    std::wstring text;
    NotePreset preset;
    glm::vec3 position;
public:
    TextNote(std::wstring text, NotePreset preset, glm::vec3 position);

    void setText(std::wstring_view text);

    const std::wstring& getText() const;

    const NotePreset& getPreset() const;

    void setPosition(const glm::vec3& position);

    const glm::vec3& getPosition() const;
};
