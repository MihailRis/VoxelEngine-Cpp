#include "Panel.h"

#include "../../graphics/Batch2D.h"

using gui::Panel;

Panel::Panel(glm::vec2 coord, glm::vec2 size, glm::vec4 color)
    : coord(coord), size(size), color(color) {

}

void Panel::draw(Batch2D* batch) {
    batch->texture(nullptr);
    batch->color = color;
    batch->rect(coord.x, coord.y, size.x, size.y);
}

bool Panel::isVisible() const {
    return visible;
}

void Panel::setVisible(bool flag) {
    visible = flag;
}