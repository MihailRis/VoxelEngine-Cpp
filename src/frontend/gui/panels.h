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
        int scroll = 0;
        int actualLength = 0;
        bool scrollable_ = true;
    public:
        Container(glm::vec2 coord, glm::vec2 size);

        void act(float delta) override;
        virtual void drawBackground(Batch2D* batch, Assets* assets) {};
        void draw(Batch2D* batch, Assets* assets) override;
        std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self) override;
        virtual void add(std::shared_ptr<UINode> node);
        virtual void add(UINode* node);
        virtual void remove(std::shared_ptr<UINode> node);
        void setScroll(int value) override;
        virtual void setScrollable(bool flag);
        void listenInterval(float interval, ontimeout callback, int repeat=-1);
        glm::vec2 contentOffset() override {return glm::vec2(0.0f, scroll);};
    };

    class Panel : public Container {
    protected:
        Orientation orientation_ = Orientation::vertical;
        glm::vec4 padding {2.0f};
        float interval = 2.0f;
        bool resizing_;
        int maxLength_ = 0;
    public:
        Panel(glm::vec2 size, glm::vec4 padding=glm::vec4(2.0f), float interval=2.0f, bool resizing=true);
        virtual ~Panel();

        void drawBackground(Batch2D* batch, Assets* assets) override;

        virtual void setOrientation(Orientation orientation);
        Orientation orientation() const;

        void refresh() override;
        void lock() override;

        virtual void setMaxLength(int value);
        int maxLength() const;
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
        Page current_;
        std::string curname_ = "";
    public:
        PagesControl();

        bool has(std::string name);
        void set(std::string name, bool history=true);
        void add(std::string name, std::shared_ptr<UINode> panel);
        void add(std::string name, UINode* panel);
        void back();
        void clearHistory();
        void reset();

        Page current();
    };
}
#endif // FRONTEND_GUI_PANELS_H_