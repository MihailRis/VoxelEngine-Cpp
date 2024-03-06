#include "controls.h"

#include <queue>
#include <sstream>
#include <iostream>
#include <algorithm>

#include "../../window/Events.h"
#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/Font.h"
#include "../../graphics/Texture.h"
#include "../../graphics/GfxContext.h"
#include "../../util/stringutil.h"
#include "GUI.h"

using namespace gui;

Label::Label(std::string text, std::string fontName) 
  : UINode(glm::vec2(text.length() * 8, 15)), 
    text(util::str2wstr_utf8(text)), 
    fontName(fontName) 
{
    setInteractive(false);
}


Label::Label(std::wstring text, std::string fontName) 
  : UINode(glm::vec2(text.length() * 8, 15)), 
    text(text), 
    fontName(fontName) 
{
    setInteractive(false);
}

void Label::setText(std::wstring text) {
    this->text = text;
    lines = 1;
    for (size_t i = 0; i < text.length(); i++) {
        if (text[i] == L'\n') {
            lines++;
        }
    }
    lines = std::max(lines, 1U);
}

const std::wstring& Label::getText() const {
    return text;
}

void Label::setFontName(std::string name) {
    this->fontName = name;
}

const std::string& Label::getFontName() const {
    return fontName;
}

void Label::setVerticalAlign(Align align) {
    this->valign = align;
}

Align Label::getVerticalAlign() const {
    return valign;
}

float Label::getLineInterval() const {
    return lineInterval;
}

void Label::setLineInterval(float interval) {
    lineInterval = interval;
}

int Label::getTextYOffset() const {
    return textYOffset;
}

size_t Label::getTextLineOffset(uint line) const {
    size_t offset = 0;
    size_t linesCount = 0;
    while (linesCount < line && offset < text.length()) {
        size_t endline = text.find(L'\n', offset);
        if (endline == std::wstring::npos) {
            break;
        }
        offset = endline+1;
        linesCount++;
    }
    return offset;
}

int Label::getLineYOffset(uint line) const {
    return line * totalLineHeight + textYOffset;
}

uint Label::getLineByYOffset(int offset) const {
    if (offset < textYOffset) {
        return 0;
    }
    return (offset - textYOffset) / totalLineHeight;
}

uint Label::getLineByTextIndex(size_t index) const {
    size_t offset = 0;
    size_t linesCount = 0;
    while (offset < index && offset < text.length()) {
        size_t endline = text.find(L'\n', offset);
        if (endline == std::wstring::npos) {
            break;
        }
        if (endline+1 > index) {
            break;
        }
        offset = endline+1;
        linesCount++;
    }
    return linesCount;
}

uint Label::getLinesNumber() const {
    return lines;
}

void Label::draw(const GfxContext* pctx, Assets* assets) {
    if (supplier) {
        setText(supplier());
    }

    auto batch = pctx->getBatch2D();
    auto font = assets->getFont(fontName);

    batch->setColor(getColor());

    uint lineHeight = font->getLineHeight();
    glm::vec2 size = getSize();
    glm::vec2 newsize (
        font->calcWidth(text), 
        (lines == 1 ? lineHeight : lineHeight*lineInterval)*lines + font->getYOffset()
    );

    glm::vec2 pos = calcPos();
    switch (align) {
        case Align::left:
            break;
        case Align::center:
            pos.x += (size.x-newsize.x)*0.5f;
            break;
        case Align::right:
            pos.x += size.x-newsize.x;
            break;
    }
    switch (valign) {
        case Align::top:
            break;
        case Align::center:
            pos.y += (size.y-newsize.y)*0.5f;
            break;
        case Align::bottom:
            pos.y += size.y-newsize.y;
            break;
    }
    textYOffset = pos.y-calcPos().y;
    totalLineHeight = lineHeight * lineInterval;

    if (multiline) {
        size_t offset = 0;
        for (uint i = 0; i < lines; i++) {
            std::wstring_view view(text.c_str()+offset, text.length()-offset);
            size_t end = view.find(L'\n');
            if (end != std::wstring::npos) {
                view = std::wstring_view(text.c_str()+offset, end);
                offset += end + 1;
            }
            font->draw(batch, view, pos.x, pos.y + i * totalLineHeight, FontStyle::none);
        }
    } else {
        font->draw(batch, text, pos.x, pos.y, FontStyle::none);
    }
}

void Label::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}


void Label::setMultiline(bool multiline) {
    this->multiline = multiline;
}

bool Label::isMultiline() const {
    return multiline;
}

// ================================= Image ====================================
Image::Image(std::string texture, glm::vec2 size) : UINode(size), texture(texture) {
    setInteractive(false);
}

void Image::draw(const GfxContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    glm::vec4 color = getColor();
    auto batch = pctx->getBatch2D();
    
    auto texture = assets->getTexture(this->texture);
    if (texture && autoresize) {
        setSize(glm::vec2(texture->width, texture->height));
    }
    batch->texture(texture);
    batch->setColor(color);
    batch->rect(pos.x, pos.y, size.x, size.y, 
                0, 0, 0, UVRegion(), false, true, color);
}

void Image::setAutoResize(bool flag) {
    autoresize = flag;
}
bool Image::isAutoResize() const {
    return autoresize;
}

// ================================= Button ===================================
Button::Button(std::shared_ptr<UINode> content, glm::vec4 padding)
    : Panel(glm::vec2(), padding, 0) {
    glm::vec4 margin = getMargin();
    setSize(content->getSize()+
            glm::vec2(padding[0]+padding[2]+margin[0]+margin[2],
                      padding[1]+padding[3]+margin[1]+margin[3]));
    add(content);
    setScrollable(false);
    setHoverColor(glm::vec4(0.05f, 0.1f, 0.15f, 0.75f));
    content->setInteractive(false);
}

Button::Button(
    std::wstring text, 
    glm::vec4 padding, 
    onaction action,
    glm::vec2 size
) : Panel(size, padding, 0) 
{
    if (size.y < 0.0f) {
        size = glm::vec2(
            glm::max(padding.x + padding.z + text.length()*8, size.x),
            glm::max(padding.y + padding.w + 16, size.y)
        );
    }
    setSize(size);

    if (action) {
        listenAction(action);
    }
    setScrollable(false);

    label = std::make_shared<Label>(text);
    label->setAlign(Align::center);
    label->setSize(size-glm::vec2(padding.z+padding.x, padding.w+padding.y));
    label->setInteractive(false);
    add(label);
    setHoverColor(glm::vec4(0.05f, 0.1f, 0.15f, 0.75f));
}

void Button::setText(std::wstring text) {
    if (label) {
        label->setText(text);
    }
}

std::wstring Button::getText() const {
    if (label) {
        return label->getText();
    }
    return L"";
}

glm::vec4 Button::getPressedColor() const {
    return pressedColor;
}

void Button::setPressedColor(glm::vec4 color) {
    pressedColor = color;
}

Button* Button::textSupplier(wstringsupplier supplier) {
    if (label) {
        label->textSupplier(supplier);
    }
    return this;
}

void Button::refresh() {
    Panel::refresh();
    if (label) {
        label->setSize(size-glm::vec2(padding.z+padding.x, padding.w+padding.y));
    }
}

void Button::drawBackground(const GfxContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(isPressed() ? pressedColor : (hover ? hoverColor : color));
    batch->rect(pos.x, pos.y, size.x, size.y);
}

void Button::mouseRelease(GUI* gui, int x, int y) {
    UINode::mouseRelease(gui, x, y);
    if (isInside(glm::vec2(x, y))) {
        for (auto callback : actions) {
            callback(gui);
        }
    }
}

Button* Button::listenAction(onaction action) {
    actions.push_back(action);
    return this;
}

void Button::setTextAlign(Align align) {
    if (label) {
        label->setAlign(align);
        refresh();
    }
}

Align Button::getTextAlign() const {
    if (label) {
        return label->getAlign();
    }
    return Align::left;
}

// ============================== RichButton ==================================
RichButton::RichButton(glm::vec2 size) : Container(size) {
    setHoverColor(glm::vec4(0.05f, 0.1f, 0.15f, 0.75f));
}

void RichButton::mouseRelease(GUI* gui, int x, int y) {
    UINode::mouseRelease(gui, x, y);
    if (isInside(glm::vec2(x, y))) {
        for (auto callback : actions) {
            callback(gui);
        }
    }
}

RichButton* RichButton::listenAction(onaction action) {
    actions.push_back(action);
    return this;
}

void RichButton::drawBackground(const GfxContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(isPressed() ? pressedColor : (hover ? hoverColor : color));
    batch->rect(pos.x, pos.y, size.x, size.y);
}

// ================================ TextBox ===================================
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

void TextBox::draw(const GfxContext* pctx, Assets* assets) {
    Panel::draw(pctx, assets);

    font = assets->getFont(label->getFontName());

    if (!isFocused())
        return;

    glm::vec2 pos = calcPos();
    glm::vec2 size = getSize();

    auto subctx = pctx->sub();
    subctx.scissors(glm::vec4(pos.x, pos.y, size.x, size.y));

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
        int startY = label->getLineYOffset(startLine);
        int endY = label->getLineYOffset(startLine);

        if (startLine == endLine) {
            batch->rect(lcoord.x + start, lcoord.y+startY, end-start, lineHeight);
        } else {
            batch->rect(lcoord.x + start, lcoord.y+endY, label->getSize().x-start-padding.z-padding.x-2, lineHeight);
            for (uint i = startLine+1; i < endLine; i++) {
                batch->rect(lcoord.x, lcoord.y+label->getLineYOffset(i), label->getSize().x-padding.z-padding.x-2, lineHeight);
            }
            batch->rect(lcoord.x, lcoord.y+label->getLineYOffset(endLine), end, lineHeight);
        }
    }
    batch->flush();
}

void TextBox::drawBackground(const GfxContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();

    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);

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
        int lineY = label->getLineYOffset(line);
        int lineHeight = font->getLineHeight() * label->getLineInterval();
        batch->rect(lcoord.x, lcoord.y+lineY, label->getSize().x, 1);
        batch->rect(lcoord.x, lcoord.y+lineY+lineHeight-2, label->getSize().x, 1);
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

void TextBox::setEditable(bool editable) {
    this->editable = editable;
}

bool TextBox::isEditable() const {
    return editable;
}

void TextBox::setOnEditStart(runnable oneditstart) {
    onEditStart = oneditstart;
}

void TextBox::focus(GUI* gui) {
    Panel::focus(gui);
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

// ============================== InputBindBox ================================
InputBindBox::InputBindBox(Binding& binding, glm::vec4 padding) 
    : Panel(glm::vec2(100,32), padding, 0),
      binding(binding) {
    label = std::make_shared<Label>(L"");
    add(label);
    setScrollable(false);
}

void InputBindBox::drawBackground(const GfxContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(isFocused() ? focusedColor : (hover ? hoverColor : color));
    batch->rect(pos.x, pos.y, size.x, size.y);
    label->setText(util::str2wstr_utf8(binding.text()));
}

void InputBindBox::clicked(GUI*, mousecode button) {
    binding.reset(button);
    defocus();
}

void InputBindBox::keyPressed(keycode key) {
    if (key != keycode::ESCAPE) {
        binding.reset(key);
    }
    defocus();
}

// ================================ TrackBar ==================================
TrackBar::TrackBar(
    double min, 
    double max, 
    double value, 
    double step, 
    int trackWidth
) : UINode(glm::vec2(26)), 
    min(min), 
    max(max), 
    value(value), 
    step(step), 
    trackWidth(trackWidth) 
{
    setColor(glm::vec4(0.f, 0.f, 0.f, 0.4f));
    setHoverColor(glm::vec4(0.01f, 0.02f, 0.03f, 0.5f));
}

void TrackBar::draw(const GfxContext* pctx, Assets* assets) {
    if (supplier) {
        value = supplier();
    }
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(hover ? hoverColor : color);
    batch->rect(pos.x, pos.y, size.x, size.y);

    float width = size.x;
    float t = (value - min) / (max-min+trackWidth*step);

    batch->setColor(trackColor);
    int actualWidth = size.x * (trackWidth / (max-min+trackWidth*step) * step);
    batch->rect(pos.x + width * t, pos.y, actualWidth, size.y);
}

void TrackBar::setSupplier(doublesupplier supplier) {
    this->supplier = supplier;
}

void TrackBar::setConsumer(doubleconsumer consumer) {
    this->consumer = consumer;
}

void TrackBar::mouseMove(GUI*, int x, int y) {
    glm::vec2 pos = calcPos();
    value = x;
    value -= pos.x;
    value = (value)/size.x * (max-min+trackWidth*step);
    value += min;
    value = (value > max) ? max : value;
    value = (value < min) ? min : value;
    value = (int)(value / step) * step;
    if (consumer) {
        consumer(value);
    }
}


double TrackBar::getValue() const {
    return value;
}

double TrackBar::getMin() const {
    return min;
}

double TrackBar::getMax() const {
    return max;
}

double TrackBar::getStep() const {
    return step;
}

int TrackBar::getTrackWidth() const {
    return trackWidth;
}

glm::vec4 TrackBar::getTrackColor() const {
    return trackColor;
}

void TrackBar::setValue(double x) {
    value = x;
}

void TrackBar::setMin(double x) {
    min = x;
}

void TrackBar::setMax(double x) {
    max = x;
}

void TrackBar::setStep(double x) {
    step = x;
}

void TrackBar::setTrackWidth(int width) {
    trackWidth = width;
}

void TrackBar::setTrackColor(glm::vec4 color) {
    trackColor = color;
}

// ================================ CheckBox ==================================
CheckBox::CheckBox(bool checked) : UINode(glm::vec2(32.0f)), checked(checked) {
    setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
}

void CheckBox::draw(const GfxContext* pctx, Assets* assets) {
    if (supplier) {
        checked = supplier();
    }
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(checked ? checkColor : (hover ? hoverColor : color));
    batch->rect(pos.x, pos.y, size.x, size.y);
}

void CheckBox::mouseRelease(GUI*, int x, int y) {
    checked = !checked;
    if (consumer) {
        consumer(checked);
    }
}

void CheckBox::setSupplier(boolsupplier supplier) {
    this->supplier = supplier;
}

void CheckBox::setConsumer(boolconsumer consumer) {
    this->consumer = consumer;
}

CheckBox* CheckBox::setChecked(bool flag) {
    checked = flag;
    return this;
}

FullCheckBox::FullCheckBox(std::wstring text, glm::vec2 size, bool checked)
    : Panel(size), 
      checkbox(std::make_shared<CheckBox>(checked)){
    setColor(glm::vec4(0.0f));
    setOrientation(Orientation::horizontal);

    add(checkbox);

    auto label = std::make_shared<Label>(text); 
    label->setMargin(glm::vec4(5.f, 5.f, 0.f, 0.f));
    add(label);
}
