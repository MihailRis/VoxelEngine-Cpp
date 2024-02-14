#ifndef FRONTEND_GUI_PANELS_H_
#define FRONTEND_GUI_PANELS_H_

#include <glm/glm.hpp>
#include <vector>
#include <stack>
#include <string>
#include <memory>
#include "UINode.h"

class Batch2D;
class Assets;

namespace gui {
    using ontimeout = std::function<void()>;

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
        int scroll = 0;
        int actualLength = 0;
        bool scrollable = true;
    public:
        Container(glm::vec2 coord, glm::vec2 size);

        virtual void act(float delta) override;
        virtual void drawBackground(const GfxContext* pctx, Assets* assets);
        virtual void draw(const GfxContext* pctx, Assets* assets) override;
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;
        virtual void addBack(std::shared_ptr<UINode> node);
        virtual void add(std::shared_ptr<UINode> node);
        virtual void add(std::shared_ptr<UINode> node, glm::vec2 coord);
        virtual void remove(std::shared_ptr<UINode> node);
        virtual void scrolled(int value) override;
        virtual void setScrollable(bool flag);
        void listenInterval(float interval, ontimeout callback, int repeat=-1);
        virtual glm::vec2 contentOffset() override {return glm::vec2(0.0f, scroll);};
        virtual void setSize(glm::vec2 size) override;
        virtual void refresh() override;

        const std::vector<std::shared_ptr<UINode>>& getNodes() const;
    };

    class Panel : public Container {
    protected:
        Orientation orientation = Orientation::vertical;
        glm::vec4 padding {2.0f};
        float interval = 2.0f;
        int maxLength = 0;
    public:
        Panel(
            glm::vec2 size, 
            glm::vec4 padding=glm::vec4(2.0f), 
            float interval=2.0f
        );
        virtual ~Panel();

        virtual void cropToContent();

        virtual void setOrientation(Orientation orientation);
        Orientation getOrientation() const;

        virtual void add(std::shared_ptr<UINode> node) override;

        virtual void refresh() override;

        virtual void setMaxLength(int value);
        int getMaxLength() const;

        virtual void setPadding(glm::vec4 padding);
        glm::vec4 getPadding() const;
    };

    struct Page {
        std::shared_ptr<UINode> panel = nullptr;

        ~Page() {
            panel = nullptr;
        }
    };

    class PagesControl : public Container {
    protected:
        std::unordered_map<std::string, Page> pages;
        std::stack<std::string> pageStack;
        Page current;
        std::string curname = "";
    public:
        PagesControl();

        bool has(std::string name);
        void setPage(std::string name, bool history=true);
        void addPage(std::string name, std::shared_ptr<UINode> panel);
        void back();
        void clearHistory();
        void reset();
    
        Page& getCurrent();
    };
}
#endif // FRONTEND_GUI_PANELS_H_