#include "panels.h"

#include <stdexcept>

#include "../../window/Window.h"
#include "../../assets/Assets.h"
#include "../../graphics/Batch2D.h"
#include "../../graphics/GfxContext.h"

using namespace gui;

using glm::vec2;
using glm::vec4;

Container::Container(vec2 coord, vec2 size) : UINode(coord, size) {
    actualLength = size.y;
}

std::shared_ptr<UINode> Container::getAt(vec2 pos, std::shared_ptr<UINode> self) {
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
    
    for (auto node : nodes) {
        if (node->isVisible()) {
            node->act(delta);
        }
    }
}

void Container::scrolled(int value) {
    int diff = (actualLength-getSize().y);
    if (diff > 0 && scrollable_) {
        scroll += value * 40;
        if (scroll > 0)
            scroll = 0;
        if (-scroll > diff) {
            scroll = -diff;
        }
    } else if (parent) {
        parent->scrolled(value);
    }
}

void Container::scrollable(bool flag) {
    scrollable_ = flag;
}

void Container::draw(const GfxContext* pctx, Assets* assets) {
    vec2 coord = calcCoord();
    vec2 size = getSize();
    drawBackground(pctx, assets);

    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->render();
    {
        GfxContext ctx = pctx->sub();
        ctx.scissors(vec4(coord.x, coord.y, size.x, size.y));
        for (auto node : nodes) {
            if (node->isVisible())
                node->draw(pctx, assets);
        }
        batch->render();
    }
}

void Container::addBack(std::shared_ptr<UINode> node) {
    nodes.insert(nodes.begin(), node);
    node->setParent(this);
    refresh();
}

void Container::add(std::shared_ptr<UINode> node) {
    nodes.push_back(node);
    node->setParent(this);
    refresh();
}

void Container::add(UINode* node) {
    add(std::shared_ptr<UINode>(node));
}

void Container::add(std::shared_ptr<UINode> node, glm::vec2 coord) {
    node->setCoord(coord);
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

Panel::Panel(vec2 size, glm::vec4 padding, float interval, bool resizing)
    : Container(vec2(), size), 
      padding(padding), 
      interval(interval), 
      resizing_(resizing) {
    setColor(vec4(0.0f, 0.0f, 0.0f, 0.75f));
}

Panel::~Panel() {
}

void Panel::drawBackground(const GfxContext* pctx, Assets* assets) {
    vec2 coord = calcCoord();

    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->color = color;
    batch->rect(coord.x, coord.y, size.x, size.y);
}

void Panel::maxLength(int value) {
    maxLength_ = value;
}

int Panel::maxLength() const {
    return maxLength_;
}

void Panel::refresh() {
    float x = padding.x;
    float y = padding.y;
    vec2 size = getSize();
    if (orientation_ == Orientation::vertical) {
        float maxw = size.x;
        for (auto& node : nodes) {
            vec2 nodesize = node->getSize();
            const vec4 margin = node->getMargin();
            y += margin.y;
            
            float ex;
            float spacex = size.x - margin.z - padding.z;
            switch (node->getAlign()) {
                case Align::center:
                    ex = x + fmax(0.0f, spacex - nodesize.x) / 2.0f;
                    break;
                case Align::right:
                    ex = x + spacex - nodesize.x;
                    break;
                default:
                    ex = x + margin.x;
            }
            node->setCoord(vec2(ex, y));
            y += nodesize.y + margin.w + interval;

            float width = size.x - padding.x - padding.z - margin.x - margin.z;
            node->setSize(vec2(width, nodesize.y));;
            node->refresh();
            maxw = fmax(maxw, ex+node->getSize().x+margin.z+padding.z);
        }
        if (resizing_) {
            if (maxLength_)
                setSize(vec2(size.x, glm::min(maxLength_, (int)(y+padding.w))));
            else
                setSize(vec2(size.x, y+padding.w));
        }
        actualLength = y + padding.w;
    } else {
        float maxh = size.y;
        for (auto& node : nodes) {
            vec2 nodesize = node->getSize();
            const vec4 margin = node->getMargin();
            x += margin.x;
            node->setCoord(vec2(x, y+margin.y));
            x += nodesize.x + margin.z + interval;
            
            float height = size.y - padding.y - padding.w - margin.y - margin.w;
            node->setSize(vec2(nodesize.x, height));
            node->refresh();
            maxh = fmax(maxh, y+margin.y+node->getSize().y+margin.w+padding.w);
        }
        if (resizing_) {
            if (maxLength_)
                setSize(vec2(glm::min(maxLength_, (int)(x+padding.z)), size.y));
            else
                setSize(vec2(x+padding.z, size.y));
        }
        actualLength = size.y;
    }
}

void Panel::orientation(Orientation orientation) {
    this->orientation_ = orientation;
}

Orientation Panel::orientation() const {
    return orientation_;
}

PagesControl::PagesControl() : Container(vec2(), vec2(1)){
}

bool PagesControl::has(std::string name) {
    return pages.find(name) != pages.end();
}

void PagesControl::add(std::string name, std::shared_ptr<UINode> panel) {
    pages[name] = Page{panel};
}

void PagesControl::add(std::string name, UINode* panel) {
    add(name, std::shared_ptr<UINode>(panel));
}

void PagesControl::set(std::string name, bool history) {
    auto found = pages.find(name);
    if (found == pages.end()) {
        throw std::runtime_error("no page found");
    }
    if (current_.panel) {
        Container::remove(current_.panel);
    }
    if (history) {
        pageStack.push(curname_);
    }
    curname_ = name;
    current_ = found->second;
    Container::add(current_.panel);
    setSize(current_.panel->getSize());
}

void PagesControl::back() {
    if (pageStack.empty())
        return;
    std::string name = pageStack.top();
    pageStack.pop();
    set(name, false);
}

Page& PagesControl::current() {
    return current_;
}

void PagesControl::clearHistory() {
    pageStack = std::stack<std::string>();
}

void PagesControl::reset() {
    clearHistory();
    if (current_.panel) {
        curname_ = "";
        Container::remove(current_.panel);
        current_ = Page{nullptr};
    }
}
