#pragma once

#include "Texture.hpp"

class GLTexture : public Texture {
protected:
    uint id;
public:
    GLTexture(uint id, uint width, uint height);
    GLTexture(const ubyte* data, uint width, uint height, ImageFormat format);
    virtual ~GLTexture();

    virtual void bind() const override;
    virtual void unbind() const override;
    virtual void reload(const ubyte* data);

    void setNearestFilter();

    virtual void reload(const ImageData& image) override;

    virtual std::unique_ptr<ImageData> readData() override;
    virtual uint getId() const override;

    virtual UVRegion getUVRegion() const override {
        return UVRegion(0.0f, 0.0f, 1.0f, 1.0f);
    }

    static std::unique_ptr<GLTexture> from(const ImageData* image);
};
