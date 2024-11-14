#include "TextNote.hpp"

TextNote::TextNote(std::wstring text, NotePreset preset, glm::vec3 position)
    : text(std::move(text)),
      preset(std::move(preset)),
      position(std::move(position)) {
}

void TextNote::setText(std::wstring_view text) {
    this->text = text;
}

const std::wstring& TextNote::getText() const {
    return text;
}

const NotePreset& TextNote::getPreset() const {
    return preset;
}

void TextNote::updatePreset(const dv::value& data) {
    preset.deserialize(data);
}

void TextNote::setPosition(const glm::vec3& position) {
    this->position = position;
}

const glm::vec3& TextNote::getPosition() const {
    return position;
}
