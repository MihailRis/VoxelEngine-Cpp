#ifndef GRAPHICS_UI_ELEMENTS_IMAGE_HPP_
#define GRAPHICS_UI_ELEMENTS_IMAGE_HPP_

#include "UINode.hpp"

namespace gui {
    class Image : public UINode {
    protected:
        std::string texture;
        bool autoresize = false;
    public:
        Image(std::string texture, glm::vec2 size=glm::vec2(32,32));

        virtual void draw(const DrawContext* pctx, Assets* assets) override;

        virtual void setAutoResize(bool flag);
        virtual bool isAutoResize() const;
        virtual const std::string& getTexture() const;
        virtual void setTexture(const std::string& name);
    };
}

#endif // GRAPHICS_UI_ELEMENTS_IMAGE_HPP_
