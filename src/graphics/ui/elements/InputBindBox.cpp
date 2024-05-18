#include "InputBindBox.hpp"

#include "Label.hpp"
#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"
#include "../../../util/stringutil.hpp"

using namespace gui;

InputBindBox::InputBindBox(Binding& binding, glm::vec4 padding) 
    : Panel(glm::vec2(100,32), padding, 0),
      binding(binding) {
    label = std::make_shared<Label>(L"");
    add(label);
    setScrollable(false);
}

void InputBindBox::drawBackground(const DrawContext* pctx, Assets*) {
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(isFocused() ? focusedColor : calcColor());
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
