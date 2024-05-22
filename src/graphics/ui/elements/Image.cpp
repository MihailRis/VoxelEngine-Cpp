#include "Image.hpp"

#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"
#include "../../core/Texture.hpp"
#include "../../../assets/Assets.hpp"
#include "../../../maths/UVRegion.hpp"

using namespace gui;

Image::Image(std::string texture, glm::vec2 size) : UINode(size), texture(texture) {
    setInteractive(false);
}

void Image::draw(const DrawContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    
    auto texture = assets->getTexture(this->texture);
    if (texture && autoresize) {
        setSize(glm::vec2(texture->getWidth(), texture->getHeight()));
    }
    batch->texture(texture);
    batch->rect(
        pos.x, pos.y, size.x, size.y, 
        0, 0, 0, UVRegion(), false, true, calcColor()
    );
}

void Image::setAutoResize(bool flag) {
    autoresize = flag;
}
bool Image::isAutoResize() const {
    return autoresize;
}

const std::string& Image::getTexture() const {
    return texture;
}

void Image::setTexture(const std::string& name) {
    texture = name;
}
