#ifndef FRONTEND_GUI_UINODE_H_
#define FRONTEND_GUI_UINODE_H_

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>

class Batch2D;
class Assets;

namespace gui {
    class UINode;
    class GUI;

    typedef std::function<void(GUI*)> onaction;
    typedef std::function<void(GUI*, double)> onnumberchange;
    
    enum class Align {
        left, center, right
    };
    class UINode {
    protected:
        glm::vec2 coord;
        glm::vec2 size_;
        glm::vec4 color_ {1.0f};
        glm::vec4 margin_ {1.0f};
        bool isvisible = true;
        bool sizelock = false;
        bool hover_ = false;
        bool pressed_ = false;
        bool focused_ = false;
        Align align_ = Align::left;
        UINode* parent = nullptr;
        UINode(glm::vec2 coord, glm::vec2 size);
    public:
        virtual ~UINode();
        virtual void act(float delta) {};
        virtual void draw(Batch2D* batch, Assets* assets) = 0;

        virtual void visible(bool flag);
        bool visible() const;

        virtual void align(Align align);
        Align align() const;

        virtual void hover(bool flag);
        bool hover() const;

        virtual void setParent(UINode* node);
        UINode* getParent() const;

        virtual void color(glm::vec4 newColor);
        glm::vec4 color() const;

        virtual void margin(glm::vec4 margin);
        glm::vec4 margin() const;

        virtual void focus(GUI*) {focused_ = true;}
        virtual void click(GUI*, int x, int y);
        virtual void clicked(GUI*, int button) {}
        virtual void mouseMove(GUI*, int x, int y) {};
        virtual void mouseRelease(GUI*, int x, int y);
        virtual void scrolled(int value);

        bool ispressed() const;
        void defocus();
        bool isfocused() const; 
        virtual bool isfocuskeeper() const {return false;}

        virtual void typed(unsigned int codepoint) {};
        virtual void keyPressed(int key) {};

        virtual bool isInside(glm::vec2 pos);
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self);

        virtual glm::vec2 contentOffset() {return glm::vec2(0.0f);};
        glm::vec2 calcCoord() const;
        void setCoord(glm::vec2 coord);
        glm::vec2 size() const;
        virtual void size(glm::vec2 size);
        void _size(glm::vec2 size);
        virtual void refresh() {};
        virtual void lock();
    };
}

#endif // FRONTEND_GUI_UINODE_H_