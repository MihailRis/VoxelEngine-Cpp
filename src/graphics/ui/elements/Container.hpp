#ifndef GRAPHICS_UI_ELEMENTS_CONTAINER_HPP_
#define GRAPHICS_UI_ELEMENTS_CONTAINER_HPP_

#include "UINode.hpp"
#include "commons.hpp"

#include <vector>

namespace gui {
    class Container : public UINode {
    protected:
        std::vector<std::shared_ptr<UINode>> nodes;
        std::vector<IntervalEvent> intervalEvents;
        int scroll = 0;
        int scrollStep = 40;
        int actualLength = 0;
        bool scrollable = true;
    public:
        Container(glm::vec2 size);

        virtual void act(float delta) override;
        virtual void drawBackground(const DrawContext* pctx, Assets* assets);
        virtual void draw(const DrawContext* pctx, Assets* assets) override;
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;
        virtual void add(std::shared_ptr<UINode> node);
        virtual void add(std::shared_ptr<UINode> node, glm::vec2 pos);
        virtual void clear();
        virtual void remove(std::shared_ptr<UINode> node);
        virtual void scrolled(int value) override;
        virtual void setScrollable(bool flag);
        void listenInterval(float interval, ontimeout callback, int repeat=-1);
        virtual glm::vec2 contentOffset() override {return glm::vec2(0.0f, scroll);};
        virtual void setSize(glm::vec2 size) override;
        virtual void refresh() override;

        const std::vector<std::shared_ptr<UINode>>& getNodes() const;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_CONTAINER_HPP_
