#include "UINode.hpp"

#include <utility>

#include "Container.hpp"
#include <graphics/core/Batch2D.hpp>

using gui::UINode;
using gui::Align;

UINode::UINode(glm::vec2 size) : size(size) {
}

UINode::~UINode() {
}

bool UINode::isVisible() const {
    if (visible && parent) {
        return parent->isVisible();
    }
    return visible;
}

void UINode::setVisible(bool flag) {
    visible = flag;
}

void UINode::setEnabled(bool flag) {
    enabled = flag;
    if (!flag) {
        defocus();
        hover = false;
    }
}

bool UINode::isEnabled() const {
    if (enabled && parent) {
        return parent->isEnabled();
    }
    return enabled;
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

UINode* UINode::listenAction(const onaction& action) {
    actions.listen(action);
    return this;
}

UINode* UINode::listenDoubleClick(const onaction& action) {
    doubleClickCallbacks.listen(action);
    return this;
}

void UINode::click(GUI*, int, int) {
    pressed = true;
}

void UINode::doubleClick(GUI* gui, int x, int y) {
    pressed = true;
    if (isInside(glm::vec2(x, y))) {
        doubleClickCallbacks.notify(gui);
    }
}

void UINode::mouseRelease(GUI* gui, int x, int y) {
    pressed = false;
    if (isInside(glm::vec2(x, y))) {
        actions.notify(gui);
    }
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

bool UINode::isInside(glm::vec2 point) {
    glm::vec2 pos = calcPos();
    glm::vec2 size = getSize();
    return (point.x >= pos.x && point.y >= pos.y && 
            point.x < pos.x + size.x && point.y < pos.y + size.y);
}

std::shared_ptr<UINode> UINode::getAt(glm::vec2 point, std::shared_ptr<UINode> self) {
    if (!isInteractive() || !enabled) {
        return nullptr;
    }
    return isInside(point) ? std::move(self) : nullptr;
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

void UINode::setTooltip(const std::wstring& text) {
    this->tooltip = text;
}

const std::wstring& UINode::getTooltip() const {
    return tooltip;
}

void UINode::setTooltipDelay(float delay) {
    tooltipDelay = delay;
}

float UINode::getTooltipDelay() const {
    return tooltipDelay;
}

glm::vec2 UINode::calcPos() const {
    if (parent) {
        return pos + parent->calcPos() + parent->contentOffset();
    }
    return pos;
}

void UINode::scrolled(int value) {
    if (parent) {
        parent->scrolled(value);
    }
}

glm::vec4 UINode::calcColor() const {
    glm::vec4 color = this->color;
    if (isEnabled()) {
        color = (isPressed() ? pressedColor : (hover ? hoverColor : color));
    } else {
        color = glm::vec4(color.r, color.g, color.b, color.a * 0.5f);
    }
    return color;
}

void UINode::setPos(glm::vec2 pos) {
    this->pos = pos;
}

glm::vec2 UINode::getPos() const {
    return pos;
}

glm::vec2 UINode::getSize() const {
    return size;
}

void UINode::setSize(glm::vec2 size) {
    this->size = glm::vec2(
        glm::max(minSize.x, size.x), glm::max(minSize.y, size.y)
    );
}

glm::vec2 UINode::getMinSize() const {
    return minSize;
}

void UINode::setMinSize(glm::vec2 minSize) {
    this->minSize = minSize;
    setSize(getSize());
}

void UINode::setColor(glm::vec4 color) {
    this->color = color;
    this->hoverColor = color;
    this->pressedColor = color;
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

glm::vec4 UINode::getPressedColor() const {
    return pressedColor;
}

void UINode::setPressedColor(glm::vec4 color) {
    pressedColor = color;
}

void UINode::setMargin(glm::vec4 margin) {
    this->margin = margin;
}

glm::vec4 UINode::getMargin() const {
    return margin;
}

void UINode::setZIndex(int zindex) {
    this->zindex = zindex;
}

int UINode::getZIndex() const {
    return zindex;
}

void UINode::moveInto(
    const std::shared_ptr<UINode>& node,
    const std::shared_ptr<Container>& dest
) {
    auto parent = node->getParent();
    if (auto container = dynamic_cast<Container*>(parent)) {
        container->remove(node);
    }
    if (parent) {
        parent->scrolled(0);
    }
    dest->add(node);
}

vec2supplier UINode::getPositionFunc() const {
    return positionfunc;
}

void UINode::setPositionFunc(vec2supplier func) {
    positionfunc = std::move(func);
}

vec2supplier UINode::getSizeFunc() const {
    return sizefunc;
}

void UINode::setSizeFunc(vec2supplier func) {
    sizefunc = std::move(func);
}

void UINode::setId(const std::string& id) {
    this->id = id;
}

const std::string& UINode::getId() const {
    return id;
}

void UINode::reposition() {
    if (positionfunc) {
        setPos(positionfunc());
    }
    if (sizefunc) {
        setSize(sizefunc());
    }
}

void UINode::setGravity(Gravity gravity) {
    if (gravity == Gravity::none) {
        setPositionFunc(nullptr);
        return;
    }
    setPositionFunc([this, gravity](){
        auto parent = getParent();
        if (parent == nullptr) {
            return getPos();
        }
        glm::vec4 margin = getMargin();
        glm::vec2 size = getSize();
        glm::vec2 parentSize = parent->getSize();

        float x = 0.0f, y = 0.0f;
        switch (gravity) {
            case Gravity::top_left:
            case Gravity::center_left:
            case Gravity::bottom_left: x = margin.x; break;
            case Gravity::top_center:
            case Gravity::center_center:
            case Gravity::bottom_center: x = (parentSize.x-size.x)/2.0f; break;
            case Gravity::top_right:
            case Gravity::center_right:
            case Gravity::bottom_right: x = parentSize.x-size.x-margin.z; break;
            default: break;
        }
        switch (gravity) {
            case Gravity::top_left:
            case Gravity::top_center:
            case Gravity::top_right: y = margin.y; break;
            case Gravity::center_left:
            case Gravity::center_center:
            case Gravity::center_right: y = (parentSize.y-size.y)/2.0f; break;
            case Gravity::bottom_left:
            case Gravity::bottom_center:
            case Gravity::bottom_right: y = parentSize.y-size.y-margin.w; break;
            default: break;
        }
        return glm::vec2(x, y);
    });

    if (parent) {
        reposition();
    }
}

bool UINode::isSubnodeOf(const UINode* node) {
    if (parent == nullptr) {
        return false;
    }
    if (parent == node) {
        return true;
    }
    return parent->isSubnodeOf(node);
}

void UINode::getIndices(
    const std::shared_ptr<UINode>& node,
    std::unordered_map<std::string, std::shared_ptr<UINode>>& map
) {
    const std::string& id = node->getId();
    if (!id.empty()) {
        map[id] = node;
    }
    auto container = std::dynamic_pointer_cast<gui::Container>(node);
    if (container) {
        for (const auto& subnode : container->getNodes()) {
            getIndices(subnode, map);
        }
    }
}
std::shared_ptr<UINode> UINode::find(
    const std::shared_ptr<UINode>& node,
    const std::string& id
) {
    if (node->getId() == id) {
        return node;
    }
    if (auto container = std::dynamic_pointer_cast<Container>(node)) {
        for (const auto& subnode : container->getNodes()) {
            if (auto found = UINode::find(subnode, id)) {
                return found;
            }
        }
    }
    return nullptr;
}
