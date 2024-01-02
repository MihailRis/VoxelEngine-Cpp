#include "controls.h"

#include <iostream>

#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/Font.h"
#include "../../util/stringutil.h"

using std::string;
using std::wstring;
using std::shared_ptr;
using glm::vec2;
using glm::vec3;
using glm::vec4;

constexpr uint KEY_ESCAPE = 256;
constexpr uint KEY_ENTER = 257;
constexpr uint KEY_BACKSPACE = 259;

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
    vec2 size = UINode::size();
    vec2 newsize = vec2(font->calcWidth(text_), font->lineHeight());
    if (newsize.x > size.x) {
        this->size(newsize);
        size = newsize;
    }
    vec2 coord = calcCoord();
    font->draw(batch, text_, coord.x, coord.y);
}

Label* Label::textSupplier(wstringsupplier supplier) {
    this->supplier = supplier;
    return this;
}

void Label::size(vec2 sizenew) {
    UINode::size(vec2(UINode::size().x, sizenew.y));
}

// ================================= Button ===================================
Button::Button(shared_ptr<UINode> content, glm::vec4 padding) : Panel(vec2(32,32), padding, 0) {
    add(content);
    scrollable(false);
}

Button::Button(wstring text, glm::vec4 padding, glm::vec4 margin) 
    : Panel(vec2(32,32), padding, 0) {
    this->margin(margin);
    Label* label = new Label(text);
    label->align(Align::center);
    this->label = shared_ptr<UINode>(label);
    add(this->label);
    scrollable(false);
}

void Button::text(std::wstring text) {
    if (label) {
        Label* label = (Label*)(this->label.get());
        label->text(text);
    }
}

wstring Button::text() const {
    if (label) {
        Label* label = (Label*)(this->label.get());
        return label->text();
    }
    return L"";
}

Button* Button::textSupplier(wstringsupplier supplier) {
    if (label) {
        Label* label = (Label*)(this->label.get());
        label->textSupplier(supplier);
    }
    return this;
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

Button* Button::listenAction(onaction action) {
    actions.push_back(action);
    return this;
}

void Button::textAlign(Align align) {
    if (label) {
        Label* label = (Label*)(this->label.get());
        label->align(align);
        refresh();
    }
}

// ================================ TextBox ===================================
TextBox::TextBox(wstring placeholder, vec4 padding) 
    : Panel(vec2(200,32), padding, 0, false), 
      input(L""),
      placeholder(placeholder) {
    label = new Label(L"");
    add(shared_ptr<UINode>(label));
}

void TextBox::drawBackground(Batch2D* batch, Assets* assets) {
    vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = (isfocused() ? focusedColor : (hover_ ? hoverColor : color_));
    batch->rect(coord.x, coord.y, size_.x, size_.y);
    if (!focused_ && supplier) {
        input = supplier();
    }

    if (input.empty()) {
        label->color(vec4(0.5f));
        label->text(placeholder);
    } else {
        label->color(vec4(1.0f));
        label->text(input);
    }
    scrollable(false);
}

void TextBox::typed(unsigned int codepoint) {
    input += wstring({(wchar_t)codepoint});
}

void TextBox::keyPressed(int key) {
    switch (key) {
        case KEY_BACKSPACE:
            if (!input.empty()){
                input = input.substr(0, input.length()-1);
            }
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

wstring TextBox::text() const {
    if (input.empty())
        return placeholder;
    return input;
}

// ============================== InputBindBox ================================
InputBindBox::InputBindBox(Binding& binding, vec4 padding) 
    : Panel(vec2(100,32), padding, 0, false),
      binding(binding) {
    label = new Label(L"");
    add(label);
    scrollable(false);
}

shared_ptr<UINode> InputBindBox::getAt(vec2 pos, shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void InputBindBox::drawBackground(Batch2D* batch, Assets* assets) {
    vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = (isfocused() ? focusedColor : (hover_ ? hoverColor : color_));
    batch->rect(coord.x, coord.y, size_.x, size_.y);
    label->text(util::str2wstr_utf8(binding.text()));
}

void InputBindBox::clicked(GUI*, int button) {
    binding.type = inputtype::mouse;
    binding.code = button;
    defocus();
}

void InputBindBox::keyPressed(int key) {
    if (key != keycode::ESCAPE) {
        binding.type = inputtype::keyboard;
        binding.code = key;
    }
    defocus();
}

// ================================ TrackBar ==================================
TrackBar::TrackBar(double min, 
                   double max, 
                   double value, 
                   double step, 
                   int trackWidth)
    : UINode(vec2(), vec2(26)), 
      min(min), 
      max(max), 
      value(value), 
      step(step), 
      trackWidth(trackWidth) {
    color(vec4(0.f, 0.f, 0.f, 0.4f));
}

void TrackBar::draw(Batch2D* batch, Assets* assets) {
    if (supplier_) {
        value = supplier_();
    }
    vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = (hover_ ? hoverColor : color_);
    batch->rect(coord.x, coord.y, size_.x, size_.y);

    float width = size_.x;
    float t = (value - min) / (max-min+trackWidth*step);

    batch->color = trackColor;
    int actualWidth = size_.x * (trackWidth / (max-min+trackWidth*step) * step);
    batch->rect(coord.x + width * t, coord.y, actualWidth, size_.y);
}

void TrackBar::supplier(doublesupplier supplier) {
    this->supplier_ = supplier;
}

void TrackBar::consumer(doubleconsumer consumer) {
    this->consumer_ = consumer;
}

void TrackBar::mouseMove(GUI*, int x, int y) {
    vec2 coord = calcCoord();
    value = x;
    value -= coord.x;
    value = (value)/size_.x * (max-min+trackWidth*step);
    value += min;
    value = (value > max) ? max : value;
    value = (value < min) ? min : value;
    value = (int)(value / step) * step;
    if (consumer_) {
        consumer_(value);
    }
}

// ================================ CheckBox ==================================
CheckBox::CheckBox(bool checked) : UINode(vec2(), vec2(32.0f)), checked_(checked) {
    color(vec4(0.0f, 0.0f, 0.0f, 0.5f));
}

void CheckBox::draw(Batch2D* batch, Assets* assets) {
    if (supplier_) {
        checked_ = supplier_();
    }
    vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = checked_ ? checkColor : (hover_ ? hoverColor : color_);
    batch->rect(coord.x, coord.y, size_.x, size_.y);
}

void CheckBox::mouseRelease(GUI*, int x, int y) {
    checked_ = !checked_;
    if (consumer_) {
        consumer_(checked_);
    }
}

void CheckBox::supplier(boolsupplier supplier) {
    supplier_ = supplier;
}

void CheckBox::consumer(boolconsumer consumer) {
    consumer_ = consumer;
}

CheckBox* CheckBox::checked(bool flag) {
    checked_ = flag;
    return this;
}