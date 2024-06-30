#include "Image.hpp"

#include <utility>

#include "../../core/DrawContext.hpp"
#include "../../core/Batch2D.hpp"
#include "../../core/Texture.hpp"
#include "../../core/Atlas.hpp"
#include "../../../assets/Assets.hpp"
#include "../../../maths/UVRegion.hpp"

using namespace gui;

Image::Image(std::string texture, glm::vec2 size) : UINode(size), texture(std::move(texture)) {
    setInteractive(false);
}

void Image::draw(const DrawContext* pctx, Assets* assets) {
    glm::vec2 pos = calcPos();
    auto batch = pctx->getBatch2D();
    
    Texture* texture = nullptr;
    auto separator = this->texture.find(':');
    if (separator == std::string::npos) {
        texture = assets->get<Texture>(this->texture);
        batch->texture(texture);
        if (texture && autoresize) {
            setSize(glm::vec2(texture->getWidth(), texture->getHeight()));
        }
    } else {
        auto atlasName = this->texture.substr(0, separator);
        if (auto atlas = assets->get<Atlas>(atlasName)) {
            texture = atlas->getTexture();
            batch->texture(atlas->getTexture());
            auto& region = atlas->get(this->texture.substr(separator+1));
            batch->setRegion(region);
            if (autoresize) {
                setSize(glm::vec2(
                    texture->getWidth()*region.getWidth(), 
                    texture->getHeight()*region.getHeight()));
            }
        }
    }
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
