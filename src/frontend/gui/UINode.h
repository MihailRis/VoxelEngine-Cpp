#ifndef FRONTEND_GUI_UINODE_H_
#define FRONTEND_GUI_UINODE_H_

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>

class GfxContext;
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
        glm::vec2 size;
        glm::vec4 color {1.0f};
        glm::vec4 margin {1.0f};
        bool visible = true;
        bool hover = false;
        bool pressed = false;
        bool focused = false;
        bool interactive = true;
        Align align = Align::left;
        UINode* parent = nullptr;
        UINode(glm::vec2 coord, glm::vec2 size);
    public:
        virtual ~UINode();
        virtual void act(float delta) {};
        virtual void draw(const GfxContext* pctx, Assets* assets) = 0;

        virtual void setVisible(bool flag);
        bool isVisible() const;

        virtual void setAlign(Align align);
        Align getAlign() const;

        virtual void setHover(bool flag);
        bool isHover() const;

        virtual void setParent(UINode* node);
        UINode* getParent() const;

        virtual void setColor(glm::vec4 newColor);
        glm::vec4 getColor() const;

        virtual void setMargin(glm::vec4 margin);
        glm::vec4 getMargin() const;

        virtual void focus(GUI*) {focused = true;}
        virtual void click(GUI*, int x, int y);
        virtual void clicked(GUI*, int button) {}
        virtual void mouseMove(GUI*, int x, int y) {};
        virtual void mouseRelease(GUI*, int x, int y);
        virtual void scrolled(int value);

        bool isPressed() const;
        void defocus();
        bool isFocused() const; 
        virtual bool isFocuskeeper() const {return false;}

        virtual void typed(unsigned int codepoint) {};
        virtual void keyPressed(int key) {};

        virtual bool isInside(glm::vec2 pos);
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self);

        virtual bool isInteractive() const;
        virtual void setInteractive(bool flag);

        virtual glm::vec2 contentOffset() {return glm::vec2(0.0f);};
        virtual glm::vec2 calcCoord() const;
        virtual void setCoord(glm::vec2 coord);
        virtual glm::vec2 getSize() const;
        virtual void setSize(glm::vec2 size);
        virtual void refresh() {};
        virtual void lock();
    };
}

#endif // FRONTEND_GUI_UINODE_H_