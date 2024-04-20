#include "Image.hpp"

#include "../../../core/GfxContext.h"
#include "../../../core/Batch2D.h"
#include "../../../core/Texture.h"
#include "../../../../assets/Assets.h"
#include "../../../../maths/UVRegion.h"

using namespace gui;

Image::Image(std::string texture, glm::vec2 size) : UINode(size), texture(texture) {
    setInteractive(false);
}

void Image::draw(const GfxContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    glm::vec4 color = getColor();
    auto batch = pctx->getBatch2D();
    
    auto texture = assets->getTexture(this->texture);
    if (texture && autoresize) {
        setSize(glm::vec2(texture->getWidth(), texture->getHeight()));
    }
    batch->texture(texture);
    batch->setColor(color);
    batch->rect(pos.x, pos.y, size.x, size.y, 
                0, 0, 0, UVRegion(), false, true, color);
}

void Image::setAutoResize(bool flag) {
    autoresize = flag;
}
bool Image::isAutoResize() const {
    return autoresize;
}
