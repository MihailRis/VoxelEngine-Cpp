#pragma once

#include "UINode.hpp"
#include "graphics/core/ImageData.hpp"
#include "graphics/core/Texture.hpp"

class Texture;

namespace gui {
    class Canvas final : public UINode {
    public:
        explicit Canvas(ImageFormat inFormat, glm::uvec2 inSize);

        ~Canvas() override = default;

        void draw(const DrawContext& pctx, const Assets& assets) override;

        [[nodiscard]] std::shared_ptr<::Texture> texture() const {
            return mTexture;
        }
    private:
        std::shared_ptr<::Texture> mTexture;
        std::unique_ptr<ImageData> mData;
    };
}