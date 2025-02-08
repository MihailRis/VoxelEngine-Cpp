#pragma once

#include "Panel.hpp"

namespace gui {
    class Label;
    
    class InputBindBox : public Panel {
    protected:
        glm::vec4 focusedColor {0.1f, 0.15f, 0.35f, 0.75f};
        std::shared_ptr<Label> label;
        Binding& binding;
    public:
        InputBindBox(Binding& binding, glm::vec4 padding=glm::vec4(6.0f));
        
        virtual void drawBackground(
            const DrawContext& pctx, const Assets& assets
        ) override;

        virtual void clicked(GUI*, mousecode button) override;
        virtual void keyPressed(keycode key) override;
        virtual bool isFocuskeeper() const override {return true;}
    };
}
