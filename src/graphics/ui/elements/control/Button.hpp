#ifndef GRAPHICS_UI_ELEMENTS_BUTTON_HPP_
#define GRAPHICS_UI_ELEMENTS_BUTTON_HPP_

#include "../layout/Panel.hpp"

namespace gui {
    class Label;

    class Button : public Panel {
    protected:
        std::shared_ptr<Label> label = nullptr;
    public:
        Button(std::shared_ptr<UINode> content, 
               glm::vec4 padding=glm::vec4(2.0f));
               
        Button(std::wstring text, 
               glm::vec4 padding,
               onaction action,
               glm::vec2 size=glm::vec2(-1));

        virtual void drawBackground(const GfxContext* pctx, Assets* assets) override;

        virtual Align getTextAlign() const;
        virtual void setTextAlign(Align align);

        virtual void setText(std::wstring text);
        virtual std::wstring getText() const;

        virtual Button* textSupplier(wstringsupplier supplier);

        virtual void refresh() override;
    };
}

#endif // GRAPHICS_UI_ELEMENTS_BUTTON_HPP_
