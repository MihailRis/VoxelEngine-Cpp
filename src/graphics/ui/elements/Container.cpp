#include "Container.hpp"

#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"

#include <algorithm>

using namespace gui;

Container::Container(glm::vec2 size) : UINode(size) {
    actualLength = size.y;
    setColor(glm::vec4());
}

std::shared_ptr<UINode> Container::getAt(glm::vec2 pos, std::shared_ptr<UINode> self) {
    if (!interactive || !isEnabled()) {
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

void Container::draw(const DrawContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    glm::vec2 size = getSize();
    drawBackground(pctx, assets);

    auto batch = pctx->getBatch2D();
    batch->texture(nullptr);
    batch->flush();
    {
        DrawContext ctx = pctx->sub();
        ctx.setScissors(glm::vec4(pos.x, pos.y, size.x, size.y));
        for (auto node : nodes) {
            if (node->isVisible())
                node->draw(pctx, assets);
        }
    }
}

void Container::drawBackground(const DrawContext* pctx, Assets*) {
    glm::vec4 color = calcColor();
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

void Container::clear() {
    for (auto node : nodes) {
        node->setParent(nullptr);
    }
    nodes.clear();
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
