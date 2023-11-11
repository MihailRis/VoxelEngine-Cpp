#include "controls.h"

#include <iostream>

#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/Font.h"

using std::string;
using std::wstring;
using std::shared_ptr;
using glm::vec2;

#define KEY_ESCAPE 256
#define KEY_ENTER 257
#define KEY_BACKSPACE 259

using namespace gui;

Label::Label(wstring text, string fontName) 
 : UINode(vec2(), vec2(text.length() * 8, 15)), text_(text), fontName_(fontName) {
}

Label& Label::text(wstring text) {
    this->text_ = text;
    return *this;
}

wstring Label::text() const {
    return text_;
}

void Label::draw(Batch2D* batch, Assets* assets) {
    if (supplier) {
        text(supplier());
    }
    batch->color = color_;
    Font* font = assets->getFont(fontName_);
    vec2 size = this->size();
    vec2 newsize = vec2(font->calcWidth(text_), font->lineHeight());
    if (newsize.x > size.x) {
        this->size(newsize);
        size = newsize;
    }
    vec2 coord = calcCoord();
    font->draw(batch, text_, coord.x, coord.y);
}

void Label::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

Button::Button(shared_ptr<UINode> content, glm::vec4 padding) : Panel(vec2(32,32), padding, 0) {
    add(content);
}

Button::Button(wstring text, glm::vec4 padding) : Panel(vec2(32,32), padding, 0) {
    Label* label = new Label(text);
    label->align(Align::center);
    add(shared_ptr<UINode>(label));
}

void Button::drawBackground(Batch2D* batch, Assets* assets) {
    vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = (ispressed() ? pressedColor : (hover_ ? hoverColor : color_));
    batch->rect(coord.x, coord.y, size_.x, size_.y);
}

shared_ptr<UINode> Button::getAt(vec2 pos, shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void Button::mouseRelease(GUI* gui, int x, int y) {
    UINode::mouseRelease(gui, x, y);
    if (isInside(vec2(x, y))) {
        for (auto callback : actions) {
            callback(gui);
        }
    }
}

void Button::listenAction(onaction action) {
    actions.push_back(action);
}

TextBox::TextBox(wstring text, vec4 padding) : Panel(vec2(200,32), padding, 0, false) {
    label = new Label(text);
    label->align(Align::center);
    add(shared_ptr<UINode>(label));
}

void TextBox::drawBackground(Batch2D* batch, Assets* assets) {
    vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = (isfocused() ? focusedColor : (hover_ ? hoverColor : color_));
    batch->rect(coord.x, coord.y, size_.x, size_.y);
    if (!focused_ && supplier) {
        label->text(supplier());
    }
}

void TextBox::typed(unsigned int codepoint) {
    label->text(label->text() + wstring({(wchar_t)codepoint}));    
}

void TextBox::keyPressed(int key) {
    wstring src = label->text();
    switch (key) {
        case KEY_BACKSPACE:
            if (src.length())
                label->text(src.substr(0, src.length()-1));
            break;
        case KEY_ENTER:
            if (consumer) {
                consumer(label->text());
            }
            defocus();
            break;
    }
}

shared_ptr<UINode> TextBox::getAt(vec2 pos, shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void TextBox::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
}

void TextBox::textConsumer(wstringconsumer consumer) {
    this->consumer = consumer;
}