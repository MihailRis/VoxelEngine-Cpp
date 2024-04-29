#include "Label.hpp"
#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"
#include "../../core/Font.hpp"
#include "../../../assets/Assets.h"
#include "../../../util/stringutil.h"

using namespace gui;

void LabelCache::prepare(Font* font, size_t wrapWidth) {
    if (font != this->font) {
        resetFlag = true;
        this->font = font;
    }
    if (wrapWidth != this->wrapWidth) {
        resetFlag = true;
        this->wrapWidth = wrapWidth;
    }
}

void LabelCache::update(const std::wstring& text, bool multiline, bool wrap) {
    resetFlag = false;
    lines.clear();
    lines.push_back(LineScheme {0, false});

    if (font == nullptr) {
        wrap = false;
    }
    size_t len = 0;
    if (multiline) {
        for (size_t i = 0; i < text.length(); i++, len++) {
            if (text[i] == L'\n') {
                lines.push_back(LineScheme {i+1, false});
                len = 0;
            } else if (i > 0 && wrap && text[i+1] != L'\n') {
                size_t width = font->calcWidth(text, i-len-1, i-(i-len)+2);
                if (width >= wrapWidth) {
                    // starting a fake line
                    lines.push_back(LineScheme {i+1, true});
                    len = 0;
                }
            }
        }
    }
}

Label::Label(std::string text, std::string fontName) 
  : UINode(glm::vec2(text.length() * 8, 15)),
    text(util::str2wstr_utf8(text)), 
    fontName(fontName) 
{
    setInteractive(false);
    cache.update(this->text, multiline, textWrap);
}


Label::Label(std::wstring text, std::string fontName) 
  : UINode(glm::vec2(text.length() * 8, 15)), 
    text(text), 
    fontName(fontName) 
{
    setInteractive(false);
    cache.update(this->text, multiline, textWrap);
}

void Label::setText(std::wstring text) {
    if (text == this->text && !cache.resetFlag) {
        return;
    }
    this->text = text;
    cache.update(this->text, multiline, textWrap);
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

bool Label::isFakeLine(size_t line) const {
    line = std::min(cache.lines.size()-1, line);
    return cache.lines.at(line).fake;
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

void Label::draw(const DrawContext* pctx, Assets* assets) {
    auto batch = pctx->getBatch2D();
    auto font = assets->getFont(fontName);
    cache.prepare(font, static_cast<size_t>(glm::abs(getSize().x)));

    if (supplier) {
        setText(supplier());
    }
    if (cache.resetFlag) {
        cache.update(text, multiline, textWrap);
    }

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
        for (size_t i = 0; i < cache.lines.size(); i++) {
            auto& line = cache.lines.at(i);
            size_t offset = line.offset;
            std::wstring_view view(text.c_str()+offset, text.length()-offset);
            if (i < cache.lines.size()-1) {
                view = std::wstring_view(text.c_str()+offset, cache.lines.at(i+1).offset-offset);
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

void Label::setTextWrapping(bool flag) {
    this->textWrap = flag;
    cache.resetFlag = true;
}

bool Label::isTextWrapping() const {
    return textWrap;
}
