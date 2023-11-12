#include "ImageData.h"

#include <assert.h>

inline int min(int a, int b) {
    return (a < b) ? a : b;
}

inline int max(int a, int b) {
    return (a > b) ? a : b;
}

ImageData::ImageData(ImageFormat format, uint width, uint height, void* data) 
    : format(format), width(width), height(height), data(data) {
}

ImageData::~ImageData() {
    switch (format) {
        case ImageFormat::rgb888:
        case ImageFormat::rgba8888:
            delete[] (ubyte*)data;
            break;
    }
}

ImageData* add_atlas_margins(ImageData* image, int grid_size) {
    // RGBA is only supported
    assert(image->getFormat() == ImageFormat::rgba8888);
    assert(image->getWidth() == image->getHeight());

    int srcwidth = image->getWidth();
    int srcheight = image->getHeight();
    int dstwidth = srcwidth + grid_size * 2;
    int dstheight = srcheight + grid_size * 2;

    const ubyte* srcdata = (const ubyte*)image->getData(); 
    ubyte* dstdata = new ubyte[dstwidth*dstheight * 4];

    int imgres = image->getWidth() / grid_size; 
    for (int row = 0; row < grid_size; row++) {
        for (int col = 0; col < grid_size; col++) {
            int ox = 1 + col * (imgres + 2);
            int oy = 1 + row * (imgres + 2);
            for (int ly = -1; ly <= imgres; ly++) {
                for (int lx = -1; lx <= imgres; lx++) {
                    int sy = max(min(ly, imgres-1), 0);
                    int sx = max(min(lx, imgres-1), 0);
                    for (int c = 0; c < 4; c++)
                        dstdata[((oy+ly) * dstwidth + ox + lx) * 4 + c] = srcdata[(sy * srcwidth + sx) * 4 + c];
                }
            }
        }
    }
}