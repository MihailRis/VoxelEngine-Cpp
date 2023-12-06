//
// Created by chelovek on 11/28/23.
//

#ifndef IMAGECUBE_H
#define IMAGECUBE_H
#include "Image.h"
#include "../../graphics-common/ITexture.h"


class ImageCube : public Image, public ITexture {
    std::vector<VkImageView> m_views;
public:
    ImageCube(int width, int height, VkFormat format);
    ~ImageCube() override;

    void bind() override;

    void reload(unsigned char* data) override;

    int getWidth() const override;

    int getHeight() const override;

};



#endif //IMAGECUBE_H
