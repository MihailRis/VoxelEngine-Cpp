#include "Panel.hpp"

using namespace gui;

Panel::Panel(glm::vec2 size, glm::vec4 padding, float interval)
  : Container(size), 
    padding(padding), 
    interval(interval) 
{
    setColor(glm::vec4(0.0f, 0.0f, 0.0f, 0.75f));
}

Panel::~Panel() {
}

void Panel::setMaxLength(int value) {
    maxLength = value;
}

int Panel::getMaxLength() const {
    return maxLength;
}

void Panel::setPadding(glm::vec4 padding) {
    this->padding = padding;
    refresh();
}

glm::vec4 Panel::getPadding() const {
    return padding;
}

void Panel::cropToContent() {
    if (maxLength > 0.0f) {
        setSize(glm::vec2(getSize().x, glm::min(maxLength, actualLength)));
    } else {
        setSize(glm::vec2(getSize().x, actualLength));
    }
}

void Panel::fullRefresh() {
    refresh();
    cropToContent();
    Container::fullRefresh();
}

void Panel::add(std::shared_ptr<UINode> node) {
    node->setResizing(true);
    Container::add(node);
    fullRefresh();
}

void Panel::refresh() {
    UINode::refresh();
    std::stable_sort(nodes.begin(), nodes.end(), [](auto a, auto b) {
        return a->getZIndex() < b->getZIndex();
    });

    float x = padding.x;
    float y = padding.y;
    glm::vec2 size = getSize();
    if (orientation == Orientation::vertical) {
        float maxw = size.x;
        for (auto& node : nodes) {
            glm::vec2 nodesize = node->getSize();
            const glm::vec4 margin = node->getMargin();
            y += margin.y;
            
            float ex = x + margin.x;
            node->setPos(glm::vec2(ex, y));
            y += nodesize.y + margin.w + interval;

            float width = size.x - padding.x - padding.z - margin.x - margin.z;
            if (node->isResizing()) {
                node->setSize(glm::vec2(width, nodesize.y));
            }
            node->refresh();
            maxw = fmax(maxw, ex+node->getSize().x+margin.z+padding.z);
        }
        actualLength = y + padding.w;
    } else {
        float maxh = size.y;
        for (auto& node : nodes) {
            glm::vec2 nodesize = node->getSize();
            const glm::vec4 margin = node->getMargin();
            x += margin.x;
            node->setPos(glm::vec2(x, y+margin.y));
            x += nodesize.x + margin.z + interval;
            
            node->refresh();
            maxh = fmax(maxh, y+margin.y+node->getSize().y+margin.w+padding.w);
        }
        actualLength = size.y;
    }
}

void Panel::setOrientation(Orientation orientation) {
    this->orientation = orientation;
}

Orientation Panel::getOrientation() const {
    return orientation;
}
