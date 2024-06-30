#ifndef GRAPHICS_CORE_IMAGE_DATA_HPP_
#define GRAPHICS_CORE_IMAGE_DATA_HPP_

#include "../../typedefs.hpp"

#include <memory>

enum class ImageFormat {
    rgb888,
    rgba8888
};

class ImageData {
    ImageFormat format;
    uint width;
    uint height;
    std::unique_ptr<ubyte[]> data;
public:
    ImageData(ImageFormat format, uint width, uint height);
    ImageData(ImageFormat format, uint width, uint height, std::unique_ptr<ubyte[]> data);
    ImageData(ImageFormat format, uint width, uint height, const ubyte* data);
    ~ImageData();

    void flipX();
    void flipY();

    void blitRGB_on_RGBA(const ImageData* image, int x, int y);
    void blitMatchingFormat(const ImageData* image, int x, int y);
    void blit(const ImageData* image, int x, int y);
    void extrude(int x, int y, int w, int h);
    void fixAlphaColor();

    ubyte* getData() const {
        return data.get();
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
