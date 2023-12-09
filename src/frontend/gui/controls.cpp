#include "controls.h"

#include <iostream>

#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/Font.h"
#include "../../util/stringutil.h"
#include "../../window/input.h"

using std::string;
using std::wstring;
using std::wstring_view;
using std::shared_ptr;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using namespace gui;

Label::Label(const wstring_view& text, const string& fontName) 
 : UINode(vec2(), vec2(text.length() * 8, 15)), text_(text), fontName_(fontName) {
}

Label& Label::text(const wstring_view& text) {
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

Label* Label::textSupplier(const wstringsupplier& supplier) {
    this->supplier = supplier;
    return this;
}

void Label::size(vec2 sizenew) {
    UINode::size(vec2(UINode::size().x, sizenew.y));
}

// ================================= Button ===================================
Button::Button(shared_ptr<UINode> content, const glm::vec4& padding) : Panel(vec2(32,32), padding, 0) {
    add(content);
    scrollable(false);
}

Button::Button(const wstring_view& text, const glm::vec4& padding) : Panel(vec2(32,32), padding, 0) {
    Label* label = new Label(text);
    label->align(Align::center);
    this->label = shared_ptr<UINode>(label);
    add(this->label);
    scrollable(false);
}

void Button::text(const std::wstring& text) {
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
        for (auto& callback : actions) {
            callback(gui);
        }
    }
}

Button* Button::listenAction(const onaction& action) {
    actions.push_back(action);
    return this;
}

// ================================ TextBox ===================================
TextBox::TextBox(const wstring& placeholder, const vec4& padding) 
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
    input += (wchar_t)codepoint;
}

void TextBox::keyPressed(int key) {
    if (key == keycode::BACKSPACE) {
        if (!input.empty()) {
            input.pop_back();
        }
    }
    else if (key == keycode::ENTER) {
        if (consumer) {
            consumer(label->text());
        }
        defocus();
    }
}

shared_ptr<UINode> TextBox::getAt(vec2 pos, shared_ptr<UINode> self) {
    return UINode::getAt(pos, self);
}

void TextBox::textSupplier(const wstringsupplier& supplier) {
    this->supplier = supplier;
}

void TextBox::textConsumer(const wstringconsumer& consumer) {
    this->consumer = consumer;
}

wstring TextBox::text() const {
    if (input.empty())
        return placeholder;
    return input;
}

// ============================== InputBindBox ================================
InputBindBox::InputBindBox(Binding& binding, const vec4& padding) 
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

void TrackBar::supplier(const doublesupplier& supplier) {
    this->supplier_ = supplier;
}

void TrackBar::consumer(const doubleconsumer& consumer) {
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

void CheckBox::supplier(const boolsupplier& supplier) {
    supplier_ = supplier;
}

void CheckBox::consumer(const boolconsumer& consumer) {
    consumer_ = consumer;
}

CheckBox* CheckBox::checked(bool flag) {
    checked_ = flag;
    return this;
}