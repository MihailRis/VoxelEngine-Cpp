#ifndef FRONTEND_GUI_PANELS_H_
#define FRONTEND_GUI_PANELS_H_

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include "UINode.h"

class Batch2D;
class Assets;

namespace gui {
    enum class Orientation { vertical, horizontal };

    class Container : public UINode {
    protected:
        std::vector<std::shared_ptr<UINode>> nodes;
    public:
        Container(glm::vec2 coord, glm::vec2 size);

        virtual void drawBackground(Batch2D* batch, Assets* assets) {};
        virtual void draw(Batch2D* batch, Assets* assets);
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;
        virtual void add(std::shared_ptr<UINode> node);
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