#include "TextNote.hpp"

TextNote::TextNote(std::wstring text, NotePreset preset, glm::vec3 position)
    : text(std::move(text)),
      preset(std::move(preset)),
      position(std::move(position)) {
}

const std::wstring& TextNote::getText() const {
    return text;
}

const NotePreset& TextNote::getPreset() const {
    return preset;
}

const glm::vec3& TextNote::getPosition() const {
    return position;
}
