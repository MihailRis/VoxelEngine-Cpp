#include "Button.hpp"

#include "Label.hpp"
#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"

using namespace gui;

Button::Button(std::shared_ptr<UINode> content, glm::vec4 padding)
    : Panel(glm::vec2(), padding, 0) {
    glm::vec4 margin = getMargin();
    Container::setSize(content->getSize()+
                       glm::vec2(padding[0]+padding[2]+margin[0]+margin[2],
                                 padding[1]+padding[3]+margin[1]+margin[3]));
    Panel::add(content);
    Container::setScrollable(false);
    UINode::setHoverColor(glm::vec4(0.05f, 0.1f, 0.15f, 0.75f));
    UINode::setPressedColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.95f));
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
    Container::setSize(size);

    if (action) {
        UINode::listenAction(action);
    }
    Container::setScrollable(false);

    label = std::make_shared<Label>(text);
    label->setAlign(Align::center);
    label->setSize(size-glm::vec2(padding.z+padding.x, padding.w+padding.y));
    label->setInteractive(false);
    Panel::add(label);
    UINode::setHoverColor(glm::vec4(0.05f, 0.1f, 0.15f, 0.75f));
    UINode::setPressedColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.95f));
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

void Button::drawBackground(const DrawContext* pctx, Assets*) {
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(calcColor());
    batch->rect(pos.x, pos.y, size.x, size.y);
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
