#ifndef GRAPHICS_CORE_TEXTURE_HPP_
#define GRAPHICS_CORE_TEXTURE_HPP_

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

    virtual void bind() = 0;
    virtual void unbind() = 0;

    virtual std::unique_ptr<ImageData> readData() = 0;

    virtual uint getWidth() const {
        return width;
    }
    virtual uint getHeight() const {
        return height;
    }
    virtual UVRegion getUVRegion() const = 0;

    virtual uint getId() const = 0;

    static std::unique_ptr<Texture> from(const ImageData* image);
};

#endif // GRAPHICS_CORE_TEXTURE_HPP_
