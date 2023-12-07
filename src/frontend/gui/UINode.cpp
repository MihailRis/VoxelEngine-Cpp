#include "UINode.h"

#include "../../graphics/Batch2D.h"

using std::shared_ptr;

using gui::UINode;
using gui::Align;

using glm::vec2;
using glm::vec4;

#include <iostream>

UINode::UINode(vec2 coord, vec2 size) : coord(coord), size_(size) {
}

UINode::~UINode() {
}

bool UINode::visible() const {
    return isvisible;
}

void UINode::setVisible(bool flag) {
    isvisible = flag;
}

Align UINode::align() const {
    return align_;
}

void UINode::setAlign(Align align) {
    align_ = align;
}

void UINode::setHover(bool flag) {
    hover_ = flag;
}

bool UINode::hover() const {
    return hover_;
}

void UINode::setParent(UINode* node) {
    parent_ = node;
}

UINode* UINode::parent() const {
    return parent_;
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
    return isInside(pos) ? self : nullptr;
}

vec2 UINode::calcCoord() const {
    if (parent_) {
        return coord + parent_->calcCoord() + parent_->contentOffset();
    }
    return coord;
}

void UINode::setScroll(int value) {
    if (parent_) {
        parent_->setScroll(value);
    }
}

void UINode::setCoord(vec2 coord) {
    this->coord = coord;
}

vec2 UINode::size() const {
    return size_;
}

void UINode::setSize(vec2 size) {
    if (sizelock)
        return;
    this->size_ = size;
}

void UINode::setColor(vec4 color) {
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