#include "containers.h"

#include <stdexcept>
#include <algorithm>

#include "../../../window/Window.h"
#include "../../../assets/Assets.h"
#include "../../core/Batch2D.h"
#include "../../core/GfxContext.h"

using namespace gui;

Container::Container(glm::vec2 size) : UINode(size) {
    actualLength = size.y;
    setColor(glm::vec4());
}

std::shared_ptr<UINode> Container::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    if (!interactive) {
        return nullptr;
    }
    if (!isInside(pos)) return nullptr;

    for (int i = nodes.size()-1; i >= 0; i--) {
        auto& node = nodes[i];
        if (!node->isVisible())
            continue;
        auto hover = node->getAt(pos, node);
        if (hover != nullptr) {
            return hover;
        }
    }
    return UINode::getAt(pos, self);
}

void Container::act(float delta) {
    for (auto node : nodes) {
        if (node->isVisible()) {
            node->act(delta);
        }
    }
    for (IntervalEvent& event : intervalEvents) {
        event.timer += delta;
        if (event.timer > event.interval) {
            event.callback();
            event.timer = fmod(event.timer, event.interval);
            if (event.repeat > 0) {
                event.repeat--;
            }
        }
    }
    intervalEvents.erase(std::remove_if(
        intervalEvents.begin(), intervalEvents.end(),
        [](const IntervalEvent& event) {
            return event.repeat == 0;
        }
    ), intervalEvents.end());
}

void Container::scrolled(int value) {
    int diff = (actualLength-getSize().y);
    if (scroll < 0 && diff <= 0) {
        scroll = 0;
    }
    if (diff > 0 && scrollable) {
        scroll += value * scrollStep;
        if (scroll > 0)
            scroll = 0;
        if (-scroll > diff) {
            scroll = -diff;
        }
    } else if (parent) {
        parent->scrolled(value);
    }
}

void Container::setScrollable(bool flag) {
    scrollable = flag;
}

void Container::draw(const GfxContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    glm::vec2 size = getSize();
    drawBackground(pctx, assets);

    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->flush();
    {
        GfxContext ctx = pctx->sub();
        ctx.setScissors(glm::vec4(pos.x, pos.y, size.x, size.y));
        for (auto node : nodes) {
            if (node->isVisible())
                node->draw(pctx, assets);
        }
    }
}

void Container::drawBackground(const GfxContext* pctx, Assets* assets) {
    glm::vec4 color = isPressed() ? pressedColor : (hover ? hoverColor : this->color);
    if (color.a <= 0.001f)
        return;
    glm::vec2 pos = calcPos();

    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->setColor(color);
    batch->rect(pos.x, pos.y, size.x, size.y);
}

void Container::add(std::shared_ptr<UINode> node) {
    nodes.push_back(node);
    node->setParent(this);
    node->reposition();
    refresh();
}

void Container::add(std::shared_ptr<UINode> node, glm::vec2 pos) {
    node->setPos(pos);
    add(node);
}

void Container::remove(std::shared_ptr<UINode> selected) {
    selected->setParent(nullptr);
    nodes.erase(std::remove_if(nodes.begin(), nodes.end(), 
        [selected](const std::shared_ptr<UINode> node) {
            return node == selected;
        }
    ), nodes.end());
    refresh();
}

void Container::listenInterval(float interval, ontimeout callback, int repeat) {
    intervalEvents.push_back({callback, interval, 0.0f, repeat});
}

void Container::setSize(glm::vec2 size) {
    if (size == getSize()) {
        refresh();
        return;
    }
    UINode::setSize(size);
    refresh();
    for (auto& node : nodes) {
        node->reposition();
    }
}

void Container::refresh() {
    std::stable_sort(nodes.begin(), nodes.end(), [](const auto& a, const auto& b) {
        return a->getZIndex() < b->getZIndex();
    });
}

const std::vector<std::shared_ptr<UINode>>& Container::getNodes() const {
    return nodes;
}

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
    Container::add(node);
    fullRefresh();
}

void Panel::refresh() {
    UINode::refresh();
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

Menu::Menu() : Container(glm::vec2(1)){
}

bool Menu::has(const std::string& name) {
    return pages.find(name) != pages.end() ||
           pageSuppliers.find(name) != pageSuppliers.end();
}

void Menu::addPage(std::string name, std::shared_ptr<UINode> panel) {
    pages[name] = Page{name, panel};
}

void Menu::addSupplier(std::string name, supplier<std::shared_ptr<UINode>> pageSupplier) {
    pageSuppliers[name] = pageSupplier;
}

void Menu::setPage(std::string name, bool history) {
    auto found = pages.find(name);
    Page page {name, nullptr};
    if (found == pages.end()) {
        auto supplier = pageSuppliers.find(name);
        if (supplier == pageSuppliers.end()) {
            throw std::runtime_error("no page found");
        } else {
            page.panel = supplier->second();
            // pages[name] = page;
            // supplied pages caching is not implemented
        }
    } else {
        page = found->second;
    }
    setPage(page, history);
}

void Menu::setPage(Page page, bool history) {
    if (current.panel) {
        Container::remove(current.panel);
    }
    if (history) {
        pageStack.push(current);
    }
    current = page;
    Container::add(current.panel);
    setSize(current.panel->getSize());
}

void Menu::back() {
    if (pageStack.empty())
        return;
    Page page = pageStack.top();
    pageStack.pop();
    setPage(page, false);
}

Page& Menu::getCurrent() {
    return current;
}

void Menu::clearHistory() {
    pageStack = std::stack<Page>();
}

void Menu::reset() {
    clearHistory();
    if (current.panel) {
        Container::remove(current.panel);
        current = Page{"", nullptr};
    }
}
