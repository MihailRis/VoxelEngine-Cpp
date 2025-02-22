#include "Canvas.hpp"

#include "graphics/core/Batch2D.hpp"
#include "graphics/core/DrawContext.hpp"
#include "graphics/core/Texture.hpp"

gui::Canvas::Canvas(ImageFormat inFormat, glm::uvec2 inSize) : UINode(inSize) {
    auto data = std::make_shared<ImageData>(inFormat, inSize.x, inSize.y);
    mTexture = Texture::from(data.get());
    mData = std::move(data);
}

void gui::Canvas::draw(const DrawContext& pctx, const Assets& assets) {
    auto pos = calcPos();
    auto col = calcColor();

    auto batch = pctx.getBatch2D();
    batch->texture(mTexture.get());
    batch->rect(pos.x, pos.y, size.x, size.y, 0, 0, 0, {}, false, true, col);
}
