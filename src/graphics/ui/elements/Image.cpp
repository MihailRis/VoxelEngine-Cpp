#include "Image.hpp"

#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"
#include "../../core/Texture.hpp"
#include "../../../assets/Assets.h"
#include "../../../maths/UVRegion.hpp"

using namespace gui;

Image::Image(std::string texture, glm::vec2 size) : UINode(size), texture(texture) {
    setInteractive(false);
}

void Image::draw(const DrawContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    glm::vec4 color = getColor();
    auto batch = pctx->getBatch2D();
    
    auto texture = assets->getTexture(this->texture);
    if (texture && autoresize) {
        setSize(glm::vec2(texture->getWidth(), texture->getHeight()));
    }
    batch->texture(texture);
    if (isEnabled()) {
        batch->setColor(isPressed() ? pressedColor : (hover ? hoverColor : color));
    } else {
        batch->setColor({color.r, color.g, color.b, color.a * 0.5f});
    }
    batch->rect(pos.x, pos.y, size.x, size.y, 
                0, 0, 0, UVRegion(), false, true, batch->getColor());
}

void Image::setAutoResize(bool flag) {
    autoresize = flag;
}
bool Image::isAutoResize() const {
    return autoresize;
}
