#include "UINode.h"

#include "../../graphics/Batch2D.h"

using gui::UINode;
using gui::Align;

UINode::UINode(glm::vec2 coord, glm::vec2 size) : coord(coord), size(size) {
}

UINode::~UINode() {
}

bool UINode::isVisible() const {
    return visible;
}

void UINode::setVisible(bool flag) {
    visible = flag;
}

Align UINode::getAlign() const {
    return align;
}

void UINode::setAlign(Align align) {
    this->align = align;
}

void UINode::setHover(bool flag) {
    hover = flag;
}

bool UINode::isHover() const {
    return hover;
}

void UINode::setParent(UINode* node) {
    parent = node;
}

UINode* UINode::getParent() const {
    return parent;
}

void UINode::click(GUI*, int x, int y) {
    pressed = true;
}

void UINode::mouseRelease(GUI*, int x, int y) {
    pressed = false;
}

bool UINode::isPressed() const {
    return pressed;
}

void UINode::defocus() {
    focused = false;
}

bool UINode::isFocused() const {
    return focused;
}

bool UINode::isInside(glm::vec2 pos) {
    glm::vec2 coord = calcCoord();
    glm::vec2 size = getSize();
    return (pos.x >= coord.x && pos.y >= coord.y && 
            pos.x < coord.x + size.x && pos.y < coord.y + size.y);
}

std::shared_ptr<UINode> UINode::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    if (!interactive) {
        return nullptr;
    }
    return isInside(pos) ? self : nullptr;
}

bool UINode::isInteractive() const {
    return interactive && isVisible();
}

void UINode::setInteractive(bool flag) {
    interactive = flag;
}

void UINode::setResizing(bool flag) {
    resizing = flag;
}

bool UINode::isResizing() const {
    return resizing;
}

glm::vec2 UINode::calcCoord() const {
    if (parent) {
        return coord + parent->calcCoord() + parent->contentOffset();
    }
    return coord;
}

void UINode::scrolled(int value) {
    if (parent) {
        parent->scrolled(value);
    }
}

void UINode::setCoord(glm::vec2 coord) {
    this->coord = coord;
}

glm::vec2 UINode::getSize() const {
    return size;
}

void UINode::setSize(glm::vec2 size) {
    this->size = size;
}

void UINode::setColor(glm::vec4 color) {
    this->color = color;
    this->hoverColor = color;
}

void UINode::setHoverColor(glm::vec4 newColor) {
    this->hoverColor = newColor;
}

glm::vec4 UINode::getHoverColor() const {
    return hoverColor;
}

glm::vec4 UINode::getColor() const {
    return color;
}

void UINode::setMargin(glm::vec4 margin) {
    this->margin = margin;
}

glm::vec4 UINode::getMargin() const {
    return margin;
}

void UINode::lock() {
}

void UINode::setId(const std::string& id) {
    this->id = id;
}

const std::string& UINode::getId() const {
    return id;
}
