#include "ImageData.h"

#include <assert.h>
#include <stdexcept>

inline int min(int a, int b) {
    return (a < b) ? a : b;
}

inline int max(int a, int b) {
    return (a > b) ? a : b;
}

ImageData::ImageData(ImageFormat format, uint width, uint height) 
    : format(format), width(width), height(height) {
    switch (format) {
        case ImageFormat::rgb888: data = new ubyte[width*height*3]{}; break;
        case ImageFormat::rgba8888: data = new ubyte[width*height*4]{}; break;
        default:
            throw std::runtime_error("format is not supported");
    }
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

void ImageData::flipX() {
    uint size;
    switch (format) {
        case ImageFormat::rgb888:
        case ImageFormat::rgba8888: {
            size = (format == ImageFormat::rgba8888) ? 4 : 3;
            ubyte* pixels = (ubyte*)data;
            for (uint y = 0; y < height; y++) {
                for (uint x = 0; x < width/2; x++) {
                    for (uint c = 0; c < size; c++) {
                        ubyte temp = pixels[(y * width + x) * size + c];
                        pixels[(y * width + x) * size + c] = pixels[(y * width + (width - x - 1)) * size + c];
                        pixels[(y * width + (width - x - 1)) * size + c] = temp;
                    }
                }
            }
            break;
        }
        default:
            throw std::runtime_error("format is not supported");
    }
}

void ImageData::flipY() {
    uint size;
    switch (format) {
        case ImageFormat::rgb888:
        case ImageFormat::rgba8888: {
            size = (format == ImageFormat::rgba8888) ? 4 : 3;
            ubyte* pixels = (ubyte*)data;
            for (uint y = 0; y < height/2; y++) {
                for (uint x = 0; x < width; x++) {
                    for (uint c = 0; c < size; c++) {
                        ubyte temp = pixels[(y * width + x) * size + c];
                        pixels[(y * width + x) * size + c] = 
                               pixels[((height-y-1) * width + x) * size + c];
                        pixels[((height-y-1) * width + x) * size + c] = temp;
                    }
                }
            }
            break;
        }
        default:
            throw std::runtime_error("format is not supported");
    }
}

void ImageData::blit(const ImageData* image, int x, int y) {
    if (format == image->format) {
        blitMatchingFormat(image, x, y);
        return;
    }
    if (format == ImageFormat::rgba8888 && 
        image->format == ImageFormat::rgb888) {
        blitRGB_on_RGBA(image, x, y);
        return;
    }
    throw std::runtime_error("mismatching format");
}

void ImageData::blitRGB_on_RGBA(const ImageData* image, int x, int y) {
    ubyte* pixels = static_cast<ubyte*>(data);
    ubyte* source = static_cast<ubyte*>(image->getData());
    uint srcwidth = image->getWidth();
    uint srcheight = image->getHeight();

    for (uint srcy = max(0, -y); (int)srcy < min(srcheight, height-y); srcy++) {
        for (uint srcx = max(0, -x); (int)srcx < min(srcwidth, width-x); srcx++) {
            uint dstx = srcx + x;
            uint dsty = srcy + y;
            uint dstidx = (dsty * width + dstx) * 4;
            uint srcidx = (srcy * srcwidth + srcx) * 3;
            for (uint c = 0; c < 3; c++) {
                pixels[dstidx + c] = source[srcidx + c];
            }
            pixels[dstidx + 3] = 255;
        }
    }
}

void ImageData::blitMatchingFormat(const ImageData* image, int x, int y) {
    uint comps;
    switch (format) {
        case ImageFormat::rgb888: comps = 3; break;
        case ImageFormat::rgba8888: comps = 4; break;
        default:
            throw std::runtime_error("only unsigned byte formats supported");    
    }
    ubyte* pixels = static_cast<ubyte*>(data);
    ubyte* source = static_cast<ubyte*>(image->getData());
    uint srcwidth = image->getWidth();
    uint srcheight = image->getHeight();

    for (uint srcy = max(0, -y); (int)srcy < min(srcheight, height-y); srcy++) {
        for (uint srcx = max(0, -x); (int)srcx < min(srcwidth, width-x); srcx++) {
            uint dstx = srcx + x;
            uint dsty = srcy + y;
            uint dstidx = (dsty * width + dstx) * comps;
            uint srcidx = (srcy * srcwidth + srcx) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = source[srcidx + c];
            }
        }
    }
}

/* Extrude rectangle zone border pixels out by 1 pixel.
   Used to remove atlas texture border artifacts */
void ImageData::extrude(int x, int y, int w, int h) {
    uint comps;
    switch (format) {
        case ImageFormat::rgb888: comps = 3; break;
        case ImageFormat::rgba8888: comps = 4; break;
        default:
            throw std::runtime_error("only unsigned byte formats supported");    
    }
    ubyte* pixels = static_cast<ubyte*>(data);

    int rx = x + w - 1;
    int ry = y + h - 1;

    // top-left pixel
    if (x > 0 && (uint)x < width && y > 0 && (uint)y < height) {
        uint srcidx = (y * width + x) * comps;
        uint dstidx = ((y - 1) * width + x - 1) * comps;
        for (uint c = 0; c < comps; c++) {
            pixels[dstidx + c] = pixels[srcidx + c];
        }
    }
    
    // top-right pixel
    if (rx >= 0 && (uint)rx < width-1 && y > 0 && (uint)y < height) {
        uint srcidx = (y * width + rx) * comps;
        uint dstidx = ((y - 1) * width + rx + 1) * comps;
        for (uint c = 0; c < comps; c++) {
            pixels[dstidx + c] = pixels[srcidx + c];
        }
    }

    // bottom-left pixel
    if (x > 0 && (uint)x < width && ry >= 0 && (uint)ry < height-1) {
        uint srcidx = (ry * width + x) * comps;
        uint dstidx = ((ry + 1) * width + x - 1) * comps;
        for (uint c = 0; c < comps; c++) {
            pixels[dstidx + c] = pixels[srcidx + c];
        }
    }
    
    // bottom-right pixel
    if (rx >= 0 && (uint)rx < width-1 && ry >= 0 && (uint)ry < height-1) {
        uint srcidx = (ry * width + rx) * comps;
        uint dstidx = ((ry + 1) * width + rx + 1) * comps;
        for (uint c = 0; c < comps; c++) {
            pixels[dstidx + c] = pixels[srcidx + c];
        }
    }

    // left border
    if (x > 0 && (uint)x < width) {
        for (uint ey = max(y, 0); (int)ey < y + h; ey++) {
            uint srcidx = (ey * width + x) * comps;
            uint dstidx = (ey * width + x - 1) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = pixels[srcidx + c];
            }
        }
    }

    // top border
    if (y > 0 && (uint)y < height) {
        for (uint ex = max(x, 0); (int)ex < x + w; ex++) {
            uint srcidx = (y * width + ex) * comps;
            uint dstidx = ((y-1) * width + ex) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = pixels[srcidx + c];
            }
        }
    }
    
    // right border
    if (rx >= 0 && (uint)rx < width-1) {
        for (uint ey = max(y, 0); (int)ey < y + h; ey++) {
            uint srcidx = (ey * width + rx) * comps;
            uint dstidx = (ey * width + rx + 1) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = pixels[srcidx + c];
            }
        }
    }

    // bottom border
    if (ry >= 0 && (uint)ry < height-1) {
        for (uint ex = max(x, 0); (int)ex < x + w; ex++) {
            uint srcidx = (ry * width + ex) * comps;
            uint dstidx = ((ry+1) * width + ex) * comps;
            for (uint c = 0; c < comps; c++) {
                pixels[dstidx + c] = pixels[srcidx + c];
            }
        }
    }
}

void ImageData::fixAlphaColor() {
    ubyte* pixels = static_cast<ubyte*>(data); 

    // Fixing black transparent pixels for Mip-Mapping
    for (uint ly = 0; ly < height-1; ly++) {
        for (uint lx = 0; lx < width-1; lx++) {
            if (pixels[((ly) * width + lx) * 4 + 3]) {
                for (int c = 0; c < 3; c++) {
                    int val = pixels[((ly) + + lx) * 4 + c];
                    if (pixels[((ly) * width + lx + 1) * 4 + 3] == 0)
                        pixels[((ly) * width + lx + 1) * 4 + c] = val;
                    if (pixels[((ly + 1) * width + lx) * 4 + 3] == 0)
                        pixels[((ly + 1) * width + lx) * 4 + c] = val;
                }
            }
        }
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
            int sox = col * imgres;
            int soy = row * imgres;
            int dox = 1 + col * (imgres + 2);
            int doy = 1 + row * (imgres + 2);
            for (int ly = -1; ly <= imgres; ly++) {
                for (int lx = -1; lx <= imgres; lx++) {
                    int sy = max(min(ly, imgres-1), 0);
                    int sx = max(min(lx, imgres-1), 0);
                    for (int c = 0; c < 4; c++)
                        dstdata[((doy+ly) * dstwidth + dox + lx) * 4 + c] = 
                           srcdata[((soy+sy) * srcwidth + sox + sx) * 4 + c];
                }
            }
            
            // Fixing black transparent pixels for Mip-Mapping
            for (int ly = 0; ly < imgres; ly++) {
                for (int lx = 0; lx < imgres; lx++) {
                    if (srcdata[((soy+ly) * srcwidth + sox + lx) * 4 + 3]) {
                        for (int c = 0; c < 3; c++) {
                            int val = srcdata[((soy+ly) * srcwidth + sox + lx) * 4 + c];
                            if (dstdata[((doy+ly) * dstwidth + dox + lx + 1) * 4 + 3] == 0)
                                dstdata[((doy+ly) * dstwidth + dox + lx + 1) * 4 + c] = val;
                            if (dstdata[((doy+ly + 1) * dstwidth + dox + lx) * 4 + 3] == 0)
                                dstdata[((doy+ly + 1) * dstwidth + dox + lx) * 4 + c] = val;
                        }
                    }
                }
            }
        }
    }
    return new ImageData(image->getFormat(), dstwidth, dstheight, dstdata);
}
