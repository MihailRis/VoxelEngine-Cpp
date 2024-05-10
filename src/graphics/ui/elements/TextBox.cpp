#include "TextBox.hpp"

#include "Label.hpp"
#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"
#include "../../core/Font.hpp"
#include "../../../assets/Assets.hpp"
#include "../../../util/stringutil.hpp"
#include "../../../window/Events.hpp"

using namespace gui;

TextBox::TextBox(std::wstring placeholder, glm::vec4 padding) 
    : Panel(glm::vec2(200,32), padding, 0), 
      input(L""),
      placeholder(placeholder) {
    label = std::make_shared<Label>(L"");
    label->setSize(size-glm::vec2(padding.z+padding.x, padding.w+padding.y));
    add(label);
    setHoverColor(glm::vec4(0.05f, 0.1f, 0.2f, 0.75f));

    textInitX = label->getPos().x;
}

void TextBox::draw(const DrawContext* pctx, Assets* assets) {
    Panel::draw(pctx, assets);

    font = assets->getFont(label->getFontName());

    if (!isFocused())
        return;

    glm::vec2 pos = calcPos();
    glm::vec2 size = getSize();

    auto subctx = pctx->sub();
    subctx.setScissors(glm::vec4(pos.x, pos.y, size.x, size.y));

    const int lineHeight = font->getLineHeight() * label->getLineInterval();
    glm::vec2 lcoord = label->calcPos();
    lcoord.y -= 2;
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    if (editable && int((Window::time() - caretLastMove) * 2) % 2 == 0) {
        uint line = label->getLineByTextIndex(caret);
        uint lcaret = caret - label->getTextLineOffset(line);
        batch->setColor(glm::vec4(1.0f));

        int width = font->calcWidth(input, lcaret);
        batch->rect(lcoord.x + width, lcoord.y+label->getLineYOffset(line), 2, lineHeight);
    }
    if (selectionStart != selectionEnd) {
        uint startLine = label->getLineByTextIndex(selectionStart);
        uint endLine = label->getLineByTextIndex(selectionEnd);

        batch->setColor(glm::vec4(0.8f, 0.9f, 1.0f, 0.25f));
        int start = font->calcWidth(input, selectionStart-label->getTextLineOffset(startLine));
        int end = font->calcWidth(input, selectionEnd-label->getTextLineOffset(endLine));
        int lineY = label->getLineYOffset(startLine);

        if (startLine == endLine) {
            batch->rect(lcoord.x + start, lcoord.y+lineY, end-start, lineHeight);
        } else {
            batch->rect(lcoord.x + start, lcoord.y+lineY, label->getSize().x-start-padding.z-padding.x-2, lineHeight);
            for (uint i = startLine+1; i < endLine; i++) {
                batch->rect(lcoord.x, lcoord.y+label->getLineYOffset(i), label->getSize().x-padding.z-padding.x-2, lineHeight);
            }
            batch->rect(lcoord.x, lcoord.y+label->getLineYOffset(endLine), end, lineHeight);
        }
    }
}

void TextBox::drawBackground(const DrawContext* pctx, Assets*) {
    glm::vec2 pos = calcPos();

    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);

    auto subctx = pctx->sub();
    subctx.setScissors(glm::vec4(pos.x, pos.y, size.x, size.y));

    if (valid) {
        if (isFocused() && !multiline) {
            batch->setColor(focusedColor);
        } else if (hover && !multiline) {
            batch->setColor(hoverColor);
        } else {
            batch->setColor(color);
        }
    } else {
        batch->setColor(invalidColor);
    }

    batch->rect(pos.x, pos.y, size.x, size.y);
    if (!isFocused() && supplier) {
        input = supplier();
    }

    if (isFocused() && multiline) {
        batch->setColor(glm::vec4(1, 1, 1, 0.1f));
        glm::vec2 lcoord = label->calcPos();
        lcoord.y -= 2;

        uint line = label->getLineByTextIndex(caret);
        while (label->isFakeLine(line)) {
            line--;
        }
        batch->setColor(glm::vec4(1, 1, 1, 0.05f));
        do {
            int lineY = label->getLineYOffset(line);
            int lineHeight = font->getLineHeight() * label->getLineInterval();

            batch->rect(lcoord.x, lcoord.y+lineY, label->getSize().x, lineHeight);
            line++;
        } while (line < label->getLinesNumber() && label->isFakeLine(line));
    }

    label->setColor(glm::vec4(input.empty() ? 0.5f : 1.0f));
    label->setText(getText());
    if (multiline && font) {
        setScrollable(true);
        uint height = label->getLinesNumber() * font->getLineHeight() * label->getLineInterval();
        label->setSize(glm::vec2(label->getSize().x, height));
        actualLength = height;
    } else {
        setScrollable(false);
    }
}

/// @brief Insert text at the caret. Also selected text will be erased
/// @param text Inserting text
void TextBox::paste(const std::wstring& text) {
    
    eraseSelected();
    if (caret >= input.length()) {
        input += text;
    } else {
        auto left = input.substr(0, caret);
        auto right = input.substr(caret);
        input = left + text + right;
    }
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
    // refresh label lines configuration for correct setCaret work
    label->setText(input);
    
    setCaret(caret + text.length());
    validate();
}

/// @brief Remove part of the text and move caret to start of the part
/// @param start start of the part
/// @param length length of part that will be removed
void TextBox::erase(size_t start, size_t length) {
    size_t end = start + length;
    if (caret > start) {
        setCaret(caret - length);
    }
    auto left = input.substr(0, start);
    auto right = input.substr(end);
    input = left + right;
}

/// @brief Remove all selected text and reset selection
/// @return true if erased anything
bool TextBox::eraseSelected() {
    if (selectionStart == selectionEnd) {
        return false;
    }
    erase(selectionStart, selectionEnd-selectionStart);
    resetSelection();
    return true;
}

void TextBox::resetSelection() {
    selectionOrigin = 0;
    selectionStart = 0;
    selectionEnd = 0;
}

void TextBox::extendSelection(int index) {
    size_t normalized = normalizeIndex(index);
    selectionStart = std::min(selectionOrigin, normalized);
    selectionEnd = std::max(selectionOrigin, normalized);
}

size_t TextBox::getLineLength(uint line) const {
    size_t position = label->getTextLineOffset(line);
    size_t lineLength = label->getTextLineOffset(line+1)-position;
    if (lineLength == 0)
        lineLength = input.length() - position + 1;
    return lineLength;
}

size_t TextBox::getSelectionLength() const {
    return selectionEnd - selectionStart;
}

/// @brief Set scroll offset
/// @param x scroll offset
void TextBox::setTextOffset(uint x) {
    label->setPos(glm::vec2(textInitX - int(x), label->getPos().y));
    textOffset = x;
}

void TextBox::typed(unsigned int codepoint) {
    paste(std::wstring({(wchar_t)codepoint}));
}

bool TextBox::validate() {
    if (validator) {
        valid = validator(getText());
    } else {
        valid = true;
    }
    return valid;
}

void TextBox::setValid(bool valid) {
    this->valid = valid;
}

bool TextBox::isValid() const {
    return valid;
}

void TextBox::setMultiline(bool multiline) {
    this->multiline = multiline;
    label->setMultiline(multiline);
    label->setVerticalAlign(multiline ? Align::top : Align::center);
}

bool TextBox::isMultiline() const {
    return multiline;
}
   
void TextBox::setTextWrapping(bool flag) {
    label->setTextWrapping(flag);
}

bool TextBox::isTextWrapping() const {
    return label->isTextWrapping();
}

void TextBox::setEditable(bool editable) {
    this->editable = editable;
}

bool TextBox::isEditable() const {
    return editable;
}

void TextBox::setOnEditStart(runnable oneditstart) {
    onEditStart = oneditstart;
}

void TextBox::onFocus(GUI* gui) {
    Panel::onFocus(gui);
    if (onEditStart){
        setCaret(input.size());
        onEditStart();
        resetSelection();
    }
}

void TextBox::refresh() {
    Panel::refresh();
    label->setSize(size-glm::vec2(padding.z+padding.x, padding.w+padding.y));
}

/// @brief Clamp index to range [0, input.length()]
/// @param index non-normalized index
/// @return normalized index
size_t TextBox::normalizeIndex(int index) {
    return std::min(input.length(), static_cast<size_t>(std::max(0, index)));
}

/// @brief Calculate index of character at defined screen X position
/// @param x screen X position
/// @param y screen Y position
/// @return non-normalized character index
int TextBox::calcIndexAt(int x, int y) const {
    if (font == nullptr)
        return 0;
    glm::vec2 lcoord = label->calcPos();
    uint line = label->getLineByYOffset(y-lcoord.y);
    line = std::min(line, label->getLinesNumber()-1);
    size_t lineLength = getLineLength(line);
    uint offset = 0;
    while (lcoord.x + font->calcWidth(input, offset) < x && offset < lineLength-1) {
        offset++;
    }
    return std::min(offset+label->getTextLineOffset(line), input.length());
}

void TextBox::click(GUI*, int x, int y) {
    int index = normalizeIndex(calcIndexAt(x, y));
    selectionStart = index;
    selectionEnd = index;
    selectionOrigin = index;
}

void TextBox::mouseMove(GUI*, int x, int y) {
    int index = calcIndexAt(x, y);
    setCaret(index);
    extendSelection(index);
    resetMaxLocalCaret();
}

void TextBox::resetMaxLocalCaret() {
    maxLocalCaret = caret - label->getTextLineOffset(label->getLineByTextIndex(caret));
}

void TextBox::performEditingKeyboardEvents(keycode key) {
    bool shiftPressed = Events::pressed(keycode::LEFT_SHIFT);
    bool breakSelection = getSelectionLength() != 0 && !shiftPressed;
    uint previousCaret = caret;
    if (key == keycode::BACKSPACE) {
        if (!eraseSelected() && caret > 0 && input.length() > 0) {
            if (caret > input.length()) {
                caret = input.length();
            }
            input = input.substr(0, caret-1) + input.substr(caret);
            setCaret(caret-1);
            validate();
        }
    } else if (key == keycode::DELETE) {
        if (!eraseSelected() && caret < input.length()) {
            input = input.substr(0, caret) + input.substr(caret + 1);
            validate();
        }
    } else if (key == keycode::ENTER) {
        if (multiline) {
            paste(L"\n");
        } else {
            if (validate() && consumer) {
                consumer(label->getText());
            }
            defocus();
        }
    } else if (key == keycode::TAB) {
        paste(L"    ");
    } else if (key == keycode::LEFT) {
        uint caret = breakSelection ? selectionStart : this->caret;
        if (caret > 0) {
            if (caret > input.length()) {
                setCaret(input.length()-1);
            } else {
                setCaret(caret-1);
            }
            if (shiftPressed) {
                if (selectionStart == selectionEnd) {
                    selectionOrigin = previousCaret;
                }
                extendSelection(this->caret);
            } else {
                resetSelection();
            }
        } else {
            setCaret(caret);
            resetSelection();
        }
        resetMaxLocalCaret();
    } else if (key == keycode::RIGHT) {
        uint caret = breakSelection ? selectionEnd : this->caret;
        if (caret < input.length()) {
            setCaret(caret+1);
            caretLastMove = Window::time();
            if (shiftPressed) {
                if (selectionStart == selectionEnd) {
                    selectionOrigin = previousCaret;
                }
                extendSelection(this->caret);
            } else {
                resetSelection();
            }
        } else {
            setCaret(caret);
            resetSelection();
        }
        resetMaxLocalCaret();
    } else if (key == keycode::UP) {
        uint caret = breakSelection ? selectionStart : this->caret;
        uint caretLine = label->getLineByTextIndex(caret);
        if (caretLine > 0) {
            uint offset = std::min(size_t(maxLocalCaret), getLineLength(caretLine-1)-1);
            setCaret(label->getTextLineOffset(caretLine-1) + offset);
        } else {
            setCaret(0);
        }
        if (shiftPressed) {
            if (selectionStart == selectionEnd) {
                selectionOrigin = previousCaret;
            }
            extendSelection(this->caret);
        } else {
            resetSelection();
        }
    } else if (key == keycode::DOWN) {
        uint caret = breakSelection ? selectionEnd : this->caret;
        uint caretLine = label->getLineByTextIndex(caret);
        if (caretLine < label->getLinesNumber()-1) {
            uint offset = std::min(size_t(maxLocalCaret), getLineLength(caretLine+1)-1);
            setCaret(label->getTextLineOffset(caretLine+1) + offset);
        } else {
            setCaret(input.length());
        }
        if (shiftPressed) {
            if (selectionStart == selectionEnd) {
                selectionOrigin = previousCaret;
            }
            extendSelection(this->caret);
        } else {
            resetSelection();
        }
    }
}

void TextBox::keyPressed(keycode key) {
    if (editable) {
        performEditingKeyboardEvents(key);
    }
    if (Events::pressed(keycode::LEFT_CONTROL)) {
        // Copy selected text to clipboard
        if (key == keycode::C || key == keycode::X) {
            std::string text = util::wstr2str_utf8(getSelection());
            if (!text.empty()) {
                Window::setClipboardText(text.c_str());
            }
            if (editable && key == keycode::X) {
                eraseSelected();
            }
        }
        // Paste text from clipboard
        if (key == keycode::V && editable) {
            const char* text = Window::getClipboardText();
            if (text) {
                paste(util::str2wstr_utf8(text));
            }
        }
        // Select/deselect all
        if (key == keycode::A) {
            if (selectionStart == selectionEnd) {
                select(0, input.length());
            } else {
                resetSelection();
            }
        }
    }
}

void TextBox::select(int start, int end) {
    if (end < start) {
        std::swap(start, end);
    }
    start = normalizeIndex(start);
    end = normalizeIndex(end);

    selectionStart = start;
    selectionEnd = end;
    selectionOrigin = start;
    setCaret(selectionEnd);
}

std::shared_ptr<UINode> TextBox::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void TextBox::setTextSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

void TextBox::setTextConsumer(wstringconsumer consumer) {
    this->consumer = consumer;
}

void TextBox::setTextValidator(wstringchecker validator) {
    this->validator = validator;
}

void TextBox::setFocusedColor(glm::vec4 color) {
    this->focusedColor = color;
}

glm::vec4 TextBox::getFocusedColor() const {
    return focusedColor;
}

void TextBox::setErrorColor(glm::vec4 color) {
    this->invalidColor = color;
}

glm::vec4 TextBox::getErrorColor() const {
    return invalidColor;
}

std::wstring TextBox::getText() const {
    if (input.empty())
        return placeholder;
    return input;
}

void TextBox::setText(const std::wstring value) {
    this->input = value;
    input.erase(std::remove(input.begin(), input.end(), '\r'), input.end());
}

std::wstring TextBox::getPlaceholder() const {
    return placeholder;
}

void TextBox::setPlaceholder(const std::wstring& placeholder) {
    this->placeholder = placeholder;
}

std::wstring TextBox::getSelection() const {
    return input.substr(selectionStart, selectionEnd-selectionStart);
}

uint TextBox::getCaret() const {
    return caret;
}

void TextBox::setCaret(uint position) {
    this->caret = std::min(size_t(position), input.length());
    caretLastMove = Window::time();

    int width = label->getSize().x;
    uint line = label->getLineByTextIndex(caret);
    int offset = label->getLineYOffset(line) + contentOffset().y;
    uint lineHeight = font->getLineHeight()*label->getLineInterval();
    scrollStep = lineHeight;
    if (offset < 0) {
        scrolled(1);
    } else if (offset >= getSize().y) {
        scrolled(-1);
    }
    uint lcaret = caret - label->getTextLineOffset(line);
    int realoffset = font->calcWidth(input, lcaret)-int(textOffset)+2;
    if (realoffset-width > 0) {
        setTextOffset(textOffset + realoffset-width);
    } else if (realoffset < 0) {
        setTextOffset(std::max(textOffset + realoffset, 0U));
    }
}
