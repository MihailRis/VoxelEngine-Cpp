#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>
#include "../typedefs.h"
#include "../graphics-base/ITexture.h"

class ImageData;

class Texture : public ITexture {
public:
	unsigned int id;
	int width;
	int height;
	Texture(unsigned int id, int width, int height);
	Texture(unsigned char* data, int width, int height, uint format);
	~Texture() override;

	void bind() override;
	void reload(unsigned char* data) override;
	int getWidth() override { return width; }

	int getHeight() override { return height; }

	static Texture* from(const ImageData* image);
};

#endif /* GRAPHICS_TEXTURE_H_ */
