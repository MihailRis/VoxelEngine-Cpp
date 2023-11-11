#include "panels.h"

#include "../../window/Window.h"
#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"

using std::shared_ptr;

using gui::UINode;
using gui::Container;
using gui::Panel;
using gui::Orientation;

using glm::vec2;
using glm::vec4;

Container::Container(vec2 coord, vec2 size) : UINode(coord, size) {
}

shared_ptr<UINode> Container::getAt(vec2 pos, shared_ptr<UINode> self) {
    for (auto node : nodes) {
        if (!node->visible())
            continue;
        auto hover = node->getAt(pos, node);
        if (hover != nullptr) {
            return hover;
        }
    }
    return UINode::getAt(pos, self);
}

void Container::draw(Batch2D* batch, Assets* assets) {
    vec2 coord = calcCoord();
    vec2 size = this->size();
    drawBackground(batch, assets);
    batch->texture(nullptr);
    batch->render();
    Window::pushScissor(vec4(coord.x, coord.y, size.x, size.y));
    for (auto node : nodes) {
        if (node->visible())
            node->draw(batch, assets);
    }
    batch->render();
    Window::popScissor();
}

void Container::add(shared_ptr<UINode> node) {
    nodes.push_back(node);
    node->setParent(this);
    refresh();
}

Panel::Panel(vec2 size, glm::vec4 padding, float interval, bool resizing)
    : Container(vec2(), size), padding(padding), interval(interval), resizing_(resizing) {
    color_ = vec4(0.0f, 0.0f, 0.0f, 0.75f);
}

Panel::~Panel() {
}

void Panel::drawBackground(Batch2D* batch, Assets* assets) {
    vec2 coord = calcCoord();
    batch->texture(nullptr);
    batch->color = color_;
    batch->rect(coord.x, coord.y, size_.x, size_.y);
}

void Panel::refresh() {
    float x = padding.x;
    float y = padding.y;
    vec2 size = this->size();
    if (orientation_ == Orientation::vertical) {
        float maxw = size.x;
        for (auto node : nodes) {
            vec2 nodesize = node->size();
            const vec4 margin = node->margin();
            y += margin.y;
            
            float ex;

            switch (node->align()) {
                case Align::center:
                    ex = x + fmax(0.0f, (size.x - margin.z - padding.z) - node->size().x) / 2.0f;
                    break;
                case Align::right:
                    ex = x + size.x - margin.z - padding.z - node->size().x;
                    break;
                default:
                    ex = x + margin.x;
            }
            node->setCoord(vec2(ex, y));
            y += nodesize.y + margin.w + interval;
            node->size(vec2(size.x - padding.x - padding.z - margin.x - margin.z, nodesize.y));
            maxw = fmax(maxw, ex+node->size().x+margin.z+padding.z);
        }
        if (resizing_)
            this->size(vec2(maxw, y+padding.w));
    } else {
        float maxh = size.y;
        for (auto node : nodes) {
            vec2 nodesize = node->size();
            const vec4 margin = node->margin();
            x += margin.x;
            node->setCoord(vec2(x, y+margin.y));
            x += nodesize.x + margin.z + interval;
            node->size(vec2(nodesize.x, size.y - padding.y - padding.w - margin.y - margin.w));
            maxh = fmax(maxh, y+margin.y+node->size().y+margin.w+padding.w);
        }
        bool increased = maxh > size.y;
        if (resizing_)
            this->size(vec2(x+padding.z, maxh));
        if (increased)
            refresh();
    }
}

void Panel::orientation(Orientation orientation) {
    this->orientation_ = orientation;
}

Orientation Panel::orientation() const {
    return orientation_;
}

void Panel::lock(){
    for (auto node : nodes) {
        node->lock();
    }
    resizing_ = false;
}