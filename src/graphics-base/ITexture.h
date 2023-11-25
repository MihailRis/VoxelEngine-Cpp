//
// Created by chelovek on 11/21/23.
//

#ifndef ITEXTURE_H
#define ITEXTURE_H

#include "../typedefs.h"

class ImageData;

class ITexture {
public:
    virtual ~ITexture() = default;

    virtual void bind() = 0;
    virtual void reload(unsigned char* data) = 0;

    virtual int getWidth() = 0;
    virtual int getHeight() = 0;

    static ITexture *from(const ImageData *data);
    static ITexture *from(unsigned char* data, int width, int height, uint format);
};



#endif //ITEXTURE_H
