#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>
#include "../typedefs.h"

class ImageData;

class Texture {
protected:
    uint id;
    uint width;
    uint height;
public:
    Texture(uint id, uint width, uint height);
    Texture(ubyte* data, uint width, uint height, uint format);
    virtual ~Texture();

    virtual void bind();
    virtual void reload(ubyte* data);

    void setNearestFilter();

    virtual ImageData* readData();

    virtual uint getWidth() const;
    virtual uint getHeight() const;

    virtual uint getId() const;

    static Texture* from(const ImageData* image);
};

#endif /* GRAPHICS_TEXTURE_H_ */
