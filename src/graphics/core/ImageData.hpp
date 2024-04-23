#ifndef GRAPHICS_CORE_IMAGE_DATA_HPP_
#define GRAPHICS_CORE_IMAGE_DATA_HPP_

#include "../../typedefs.h"

enum class ImageFormat {
    rgb888,
    rgba8888
};

class ImageData {
    ImageFormat format;
    uint width;
    uint height;
    void* data;
public:
    ImageData(ImageFormat format, uint width, uint height);
    ImageData(ImageFormat format, uint width, uint height, void* data);
    ~ImageData();

    void flipX();
    void flipY();

    void blitRGB_on_RGBA(const ImageData* image, int x, int y);
    void blitMatchingFormat(const ImageData* image, int x, int y);
    void blit(const ImageData* image, int x, int y);
    void extrude(int x, int y, int w, int h);
    void fixAlphaColor();

    void* getData() const {
        return data;
    }

    ImageFormat getFormat() const {
        return format;
    }

    uint getWidth() const {
        return width;
    }

    uint getHeight() const {
        return height;
    }
};

extern ImageData* add_atlas_margins(ImageData* image, int grid_size);

#endif // GRAPHICS_CORE_IMAGE_DATA_HPP_
