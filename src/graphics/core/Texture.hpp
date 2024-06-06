#ifndef GRAPHICS_CORE_TEXTURE_HPP_
#define GRAPHICS_CORE_TEXTURE_HPP_

#include "../../typedefs.hpp"
#include "ImageData.hpp"

#include <memory>

class Texture {
protected:
    uint id;
    uint width;
    uint height;
public:
    static uint MAX_RESOLUTION;

    Texture(uint id, uint width, uint height);
    Texture(ubyte* data, uint width, uint height, ImageFormat format);
    virtual ~Texture();

    virtual void bind();
    virtual void unbind();
    virtual void reload(ubyte* data);

    void setNearestFilter();

    virtual std::unique_ptr<ImageData> readData();

    [[nodiscard]] virtual uint getWidth() const;
    [[nodiscard]] virtual uint getHeight() const;

    [[nodiscard]] virtual uint getId() const;

    static std::unique_ptr<Texture> from(const ImageData* image);
};

#endif // GRAPHICS_CORE_TEXTURE_HPP_
