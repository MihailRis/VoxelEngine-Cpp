#include "SplitBox.hpp"

using namespace gui;

SplitBox::SplitBox(const glm::vec2& size, float splitPos, Orientation orientation)
 : Container(size), splitPos(splitPos), orientation(orientation) {
    setCursor(
        orientation == Orientation::vertical ? CursorShape::NS_RESIZE
                                             : CursorShape::EW_RESIZE
    );
}

void SplitBox::mouseMove(GUI*, int x, int y) {
    auto pos = calcPos();
    auto size = getSize();
    
    glm::ivec2 cursor(x - pos.x, y - pos.y);
    int axis = orientation == Orientation::vertical;

    int v = cursor[axis];
    v = std::max(std::min(static_cast<int>(size[axis]) - 10, v), 10);
    float t = v / size[axis];
    splitPos = t;

    refresh();
}

void SplitBox::refresh() {
    Container::refresh();

    if (nodes.empty()) {
        return;
    }
    glm::vec2 size = getSize();
    if (nodes.size() == 1) {
        auto node = nodes.at(0);
        node->setPos(glm::vec2());
        node->setSize(size);
        return;
    }
    auto nodeA = nodes.at(0);
    auto nodeB = nodes.at(1);
    
    nodeA->setPos(glm::vec2());
    if (orientation == Orientation::vertical) {
        float splitPos = this->splitPos * size.y;
        nodeA->setSize(glm::vec2(size.x, splitPos - splitRadius));
        nodeB->setSize(glm::vec2(size.x, size.y - splitPos - splitRadius));
        nodeB->setPos(glm::vec2(0.0f, splitPos + splitRadius));
    } else {
        float splitPos = this->splitPos * size.x;
        nodeA->setSize(glm::vec2(splitPos - splitRadius, size.y));
        nodeB->setSize(glm::vec2(size.x - splitPos - splitRadius, size.y));
        nodeB->setPos(glm::vec2(splitPos + splitRadius, 0.0f));
    }
}

void SplitBox::fullRefresh() {
    refresh();
    reposition();
    Container::fullRefresh();
}
