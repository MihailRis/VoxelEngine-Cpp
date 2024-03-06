#ifndef FRONTEND_GUI_UINODE_H_
#define FRONTEND_GUI_UINODE_H_

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include "../../delegates.h"
#include "../../window/input.h"

class GfxContext;
class Assets;

namespace gui {
    class UINode;
    class GUI;

    using onaction = std::function<void(GUI*)>;
    using onnumberchange = std::function<void(GUI*, double)>;
    
    enum class Align {
        left, center, right,
        top=left, bottom=right,
    };

    class UINode {
        /** 
         * element identifier used for direct access in UiDocument
        */
        std::string id = "";
    protected:
        glm::vec2 coord {0.0f};
        glm::vec2 size;
        glm::vec2 minSize {1.0f};
        glm::vec4 color {1.0f};
        glm::vec4 hoverColor {1.0f};
        glm::vec4 margin {1.0f};
        bool visible = true;
        bool hover = false;
        bool pressed = false;
        bool focused = false;
        bool interactive = true;
        bool resizing = true;
        int zindex = 0;
        Align align = Align::left;
        UINode* parent = nullptr;
        vec2supplier positionfunc = nullptr;
        UINode(glm::vec2 size);
    public:
        virtual ~UINode();
        /** Called every frame for all visible elements 
         * @param delta delta time
        */
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

        /** Set element color (doesn't affect inner elements).
           Also replaces hover color to avoid adding extra properties. */
        virtual void setColor(glm::vec4 newColor);

        /** Get element color (float R,G,B,A in range [0.0, 1.0])*/
        glm::vec4 getColor() const;

        virtual void setHoverColor(glm::vec4 newColor);
        glm::vec4 getHoverColor() const;

        virtual void setMargin(glm::vec4 margin);
        glm::vec4 getMargin() const;

        /** Influences container elements sort order 
            Doesn't work in Panel */
        virtual void setZIndex(int idx);
        int getZIndex() const;

        virtual void focus(GUI*) {focused = true;}
        virtual void click(GUI*, int x, int y);
        virtual void clicked(GUI*, mousecode button) {}
        virtual void mouseMove(GUI*, int x, int y) {};
        virtual void mouseRelease(GUI*, int x, int y);
        virtual void scrolled(int value);

        bool isPressed() const;
        void defocus();
        bool isFocused() const; 

        /** Check if element catches all user input when focused */
        virtual bool isFocuskeeper() const {return false;}

        virtual void typed(unsigned int codepoint) {};
        virtual void keyPressed(keycode key) {};

        /** Check if screen position is inside of the element 
          * @param pos screen position */
        virtual bool isInside(glm::vec2 pos);

        /** Get element under the cursor.
         *  @param pos cursor screen position
         *  @param self shared pointer to element
         *  @return self, sub-element or nullptr if element is not interractive */
        virtual std::shared_ptr<UINode> getAt(glm::vec2 pos, std::shared_ptr<UINode> self);

        /* Check if element is opaque for cursor */
        virtual bool isInteractive() const;
        /* Make the element opaque (true) or transparent (false) for cursor */
        virtual void setInteractive(bool flag);

        virtual void setResizing(bool flag);
        virtual bool isResizing() const;

        /* Get inner content offset. Used for scroll */
        virtual glm::vec2 contentOffset() {return glm::vec2(0.0f);};
        /* Calculate screen position of the element */
        virtual glm::vec2 calcCoord() const;
        virtual void setCoord(glm::vec2 coord);
        virtual glm::vec2 getCoord() const;
        virtual glm::vec2 getSize() const;
        virtual void setSize(glm::vec2 size);
        virtual glm::vec2 getMinSize() const;
        virtual void setMinSize(glm::vec2 size);
        /* Called in containers when new element added */
        virtual void refresh() {};
        virtual void lock();

        virtual vec2supplier getPositionFunc() const;
        virtual void setPositionFunc(vec2supplier);

        void setId(const std::string& id);
        const std::string& getId() const;

        /* Fetch coord from positionfunc if assigned */
        void reposition();
    };
}

#endif // FRONTEND_GUI_UINODE_H_