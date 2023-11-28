//
// Created by chelovek on 11/28/23.
//

#ifndef IMAGECUBE_H
#define IMAGECUBE_H
#include "Image.h"
#include "../../graphics-base/ITexture.h"


class ImageCube : public Image, public ITexture {
public:
    ImageCube(int width, int height, VkFormat format);

    void bind() override;

    void reload(unsigned char* data) override;

    int getWidth() override;

    int getHeight() override;
};



#endif //IMAGECUBE_H
