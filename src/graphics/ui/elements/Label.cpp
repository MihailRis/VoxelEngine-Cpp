#include "Label.hpp"

#include "../../core/GfxContext.h"
#include "../../core/Batch2D.h"
#include "../../core/Font.h"
#include "../../../assets/Assets.h"
#include "../../../util/stringutil.h"

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
