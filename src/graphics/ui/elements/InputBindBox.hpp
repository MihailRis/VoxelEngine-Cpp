#ifndef GRAPHICS_UI_ELEMENTS_INPUTBINDBOX_HPP_
#define GRAPHICS_UI_ELEMENTS_INPUTBINDBOX_HPP_

#include "Panel.hpp"

namespace gui {
    class Label;
    
    class InputBindBox : public Panel {
    protected:
        glm::vec4 hoverColor {0.05f, 0.1f, 0.2f, 0.75f};
        glm::vec4 focusedColor {0.0f, 0.0f, 0.0f, 1.0f};
        std::shared_ptr<Label> label;
        Binding& binding;
    public:
        InputBindBox(Binding& binding, glm::vec4 padding=glm::vec4(6.0f));
        virtual void drawBackground(const GfxContext* pctx, Assets* assets) override;

        virtual void clicked(GUI*, mousecode button) override;
        virtual void keyPressed(keycode key) override;
        virtual bool isFocuskeeper() const override {return true;}
    };
}

#endif // GRAPHICS_UI_ELEMENTS_INPUTBINDBOX_HPP_
