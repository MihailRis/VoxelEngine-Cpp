#ifndef FRONTEND_GUI_PANELS_H_
#define FRONTEND_GUI_PANELS_H_

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "UINode.h"

class Batch2D;
class Assets;

namespace gui {
    typedef std::function<void()> ontimeout;
    struct IntervalEvent {
        ontimeout callback;
        float interval;
        float timer;
        // -1 - infinity, 1 - one time event
        int repeat;
    };

    enum class Orientation { vertical, horizontal };

    class Container : public UINode {
    protected:
        std::vector<std::shared_ptr<UINode>> nodes;
        std::vector<IntervalEvent> intervalEvents;
    public:
        Container(glm::vec2 coord, glm::vec2 size);

        virtual void act(float delta) override;
        virtual void drawBackground(Batch2D* batch, Assets* assets) {};
        virtual void draw(Batch2D* batch, Assets* assets) override;
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;
        virtual void add(std::shared_ptr<UINode> node);
        virtual void remove(std::shared_ptr<UINode> node);
        void listenInterval(float interval, ontimeout callback, int repeat=-1);
    };

    class Panel : public Container {
    protected:
        Orientation orientation_ = Orientation::vertical;
        glm::vec4 padding {2.0f};
        float interval = 2.0f;
        bool resizing_;
    public:
        Panel(glm::vec2 size, glm::vec4 padding=glm::vec4(2.0f), float interval=2.0f, bool resizing=true);
        virtual ~Panel();

        virtual void drawBackground(Batch2D* batch, Assets* assets) override;

        virtual void orientation(Orientation orientation);
        Orientation orientation() const;

        virtual void refresh() override;
        virtual void lock() override;
    };
}
#endif // FRONTEND_GUI_PANELS_H_