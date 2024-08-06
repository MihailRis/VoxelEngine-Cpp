#include "CheckBox.hpp"

#include <utility>

#include <graphics/core/DrawContext.hpp>
#include <graphics/core/Batch2D.hpp>
#include "Label.hpp"

using namespace gui;

CheckBox::CheckBox(bool checked) : UINode(glm::vec2(32.0f)), checked(checked) {
    setColor({0.0f, 0.0f, 0.0f, 0.5f});
    setHoverColor({0.05f, 0.1f, 0.2f, 0.75f});
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
    this->supplier = std::move(supplier);
}

void CheckBox::setConsumer(boolconsumer consumer) {
    this->consumer = std::move(consumer);
}

CheckBox* CheckBox::setChecked(bool flag) {
    checked = flag;
    return this;
}

FullCheckBox::FullCheckBox(const std::wstring& text, glm::vec2 size, bool checked)
    : Panel(size), 
      checkbox(std::make_shared<CheckBox>(checked)){
    setColor(glm::vec4(0.0f));
    setOrientation(Orientation::horizontal);

    add(checkbox);

    auto label = std::make_shared<Label>(text); 
    label->setMargin(glm::vec4(5.f, 5.f, 0.f, 0.f));
    add(label);
}
