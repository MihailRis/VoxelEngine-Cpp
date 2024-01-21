#include "UINode.h"

#include "../../graphics/Batch2D.h"

using std::shared_ptr;

using gui::UINode;
using gui::Align;

using glm::vec2;
using glm::vec4;

UINode::UINode(vec2 coord, vec2 size) : coord(coord), size_(size) {
}

UINode::~UINode() {
}

bool UINode::visible() const {
    return isvisible;
}

void UINode::visible(bool flag) {
    isvisible = flag;
}

Align UINode::align() const {
    return align_;
}

void UINode::align(Align align) {
    align_ = align;
}

void UINode::hover(bool flag) {
    hover_ = flag;
}

bool UINode::hover() const {
    return hover_;
}

void UINode::setParent(UINode* node) {
    parent = node;
}

UINode* UINode::getParent() const {
    return parent;
}

void UINode::click(GUI*, int x, int y) {
    pressed_ = true;
}

void UINode::mouseRelease(GUI*, int x, int y) {
    pressed_ = false;
}

bool UINode::ispressed() const {
    return pressed_;
}

void UINode::defocus() {
    focused_ = false;
}

bool UINode::isfocused() const {
    return focused_;
}

bool UINode::isInside(glm::vec2 pos) {
    vec2 coord = calcCoord();
    vec2 size = this->size();
    return (pos.x >= coord.x && pos.y >= coord.y && 
            pos.x < coord.x + size.x && pos.y < coord.y + size.y);
}

shared_ptr<UINode> UINode::getAt(vec2 pos, shared_ptr<UINode> self) {
    if (!interactive) {
        return nullptr;
    }
    return isInside(pos) ? self : nullptr;
}

bool UINode::isInteractive() const {
    return interactive && visible();
}

void UINode::setInteractive(bool flag) {
    interactive = flag;
}

vec2 UINode::calcCoord() const {
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

void UINode::setCoord(vec2 coord) {
    this->coord = coord;
}

vec2 UINode::size() const {
    return size_;
}

void UINode::size(vec2 size) {
    if (sizelock)
        return;
    this->size_ = size;
}

void UINode::_size(vec2 size) {
    if (sizelock)
        return;
    this->size_ = size;
}

void UINode::color(vec4 color) {
    this->color_ = color;
}

vec4 UINode::color() const {
    return color_;
}

void UINode::margin(vec4 margin) {
    this->margin_ = margin;
}

vec4 UINode::margin() const {
    return margin_;
}

void UINode::lock() {
}