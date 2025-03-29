#pragma once

#include "typedefs.hpp"
#include "maths/UVRegion.hpp"
#include "ImageData.hpp"

#include <memory>

class Texture {
protected:
    uint width;
    uint height;

    Texture(uint width, uint height) : width(width), height(height) {}
public:
    static uint MAX_RESOLUTION;

    virtual ~Texture() {}

    virtual void bind() const = 0;
    virtual void unbind() const = 0;

    virtual void reload(const ImageData& image) = 0;

    virtual std::unique_ptr<ImageData> readData() = 0;

    virtual uint getWidth() const {
        return width;
    }
    virtual uint getHeight() const {
        return height;
    }
    virtual UVRegion getUVRegion() const = 0;

    virtual uint getId() const = 0;

    virtual void setMipMapping(bool flag, bool pixelated) = 0;

    static std::unique_ptr<Texture> from(const ImageData* image);
};
