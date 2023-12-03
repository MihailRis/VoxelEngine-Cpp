#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>
#include "../typedefs.h"
#include "../graphics-base/ITexture.h"

class ImageData;

class Texture : public ITexture {
public:
	uint id;
	int width;
	int height;
	Texture(uint id, int width, int height);
	Texture(ubyte* data, int width, int height, uint format);
	~Texture() override;

	void bind() override;
	void reload(ubyte* data) override;
	int getWidth() const override { return width; }

	int getHeight() const override { return height; }

	static Texture* from(const ImageData* image);
};

#endif /* GRAPHICS_TEXTURE_H_ */
