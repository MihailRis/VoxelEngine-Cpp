//
// Created by chelovek on 11/19/23.
//

#ifndef TEXTURE_H
#define TEXTURE_H

#include "Image.h"
#include "../../graphics-common/ITexture.h"

class ImageData;

class Image2d final : public Image, public ITexture {
public:
    Image2d(const unsigned char* data, int width, int height, VkFormat format);

    void bind() override;

    void reload(unsigned char* data) override;

    static Image2d *from(const ImageData *data);

    int getWidth() const override;

    int getHeight() const override;
};

namespace vulkan {
    using Texture = Image2d;
}

#endif //TEXTURE_H
