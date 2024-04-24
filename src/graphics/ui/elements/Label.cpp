#include "Label.hpp"
#include "../../core/GfxContext.hpp"
#include "../../core/Batch2D.hpp"
#include "../../core/Font.hpp"
#include "../../../assets/Assets.h"
#include "../../../util/stringutil.h"

using namespace gui;

void LabelCache::update(const std::wstring& text, bool multiline) {
    resetFlag = false;
    lines.clear();

    if (multiline) {
        lines.push_back(LineScheme {0});
        for (size_t i = 0; i < text.length(); i++) {
            if (text[i] == L'\n') {
                lines.push_back(LineScheme {i+1});
            }
        }
    }
    if (lines.empty()) {
        lines.push_back(LineScheme {0});
    }
}

Label::Label(std::string text, std::string fontName) 
  : UINode(glm::vec2(text.length() * 8, 15)), 
    text(util::str2wstr_utf8(text)), 
    fontName(fontName) 
{
    setInteractive(false);
    cache.update(this->text, multiline);
}


Label::Label(std::wstring text, std::string fontName) 
  : UINode(glm::vec2(text.length() * 8, 15)), 
    text(text), 
    fontName(fontName) 
{
    setInteractive(false);
    cache.update(this->text, multiline);
}

void Label::setText(std::wstring text) {
    if (text == this->text && !cache.resetFlag) {
        return;
    }
    this->text = text;
    cache.update(this->text, multiline);
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

size_t Label::getTextLineOffset(size_t line) const {
    line = std::min(cache.lines.size()-1, line);
    return cache.lines.at(line).offset;
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
    for (size_t i = 0; i < cache.lines.size(); i++) {
        if (cache.lines.at(i).offset > index) {
            return i-1;
        }
    }
    return cache.lines.size()-1;
}

uint Label::getLinesNumber() const {
    return cache.lines.size();
}

void Label::draw(const GfxContext* pctx, Assets* assets) {
    if (supplier) {
        setText(supplier());
    }

    auto batch = pctx->getBatch2D();
    auto font = assets->getFont(fontName);

    batch->setColor(getColor());

    uint lineHeight = font->getLineHeight();
    if (cache.lines.size() > 1) {
        lineHeight *= lineInterval;
    }
    glm::vec2 size = getSize();
    glm::vec2 newsize (
        font->calcWidth(text), 
        lineHeight * cache.lines.size() + font->getYOffset()
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
    totalLineHeight = lineHeight;

    if (multiline) {
        size_t offset = 0;
        for (uint i = 0; i < cache.lines.size(); i++) {
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
    if (multiline != this->multiline) {
        this->multiline = multiline;
        cache.resetFlag = true;
    }
}

bool Label::isMultiline() const {
    return multiline;
}
