#include "CheckBox.hpp"

#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"
#include "Label.hpp"

using namespace gui;

CheckBox::CheckBox(bool checked) : UINode(glm::vec2(32.0f)), checked(checked) {
    UINode::setColor({0.0f, 0.0f, 0.0f, 0.5f});
    UINode::setHoverColor({0.05f, 0.1f, 0.2f, 0.75f});
}

void CheckBox::draw(const DrawContext* pctx, Assets*) {
    if (supplier) {
        checked = supplier();
    }
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(checked ? checkColor : calcColor());
    batch->rect(pos.x, pos.y, size.x, size.y);
}

void CheckBox::mouseRelease(GUI*, int, int) {
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
    UINode::setColor(glm::vec4(0.0f));
    Panel::setOrientation(Orientation::horizontal);

    Panel::add(checkbox);

    auto label = std::make_shared<Label>(text); 
    label->setMargin(glm::vec4(5.f, 5.f, 0.f, 0.f));
    Panel::add(label);
}
