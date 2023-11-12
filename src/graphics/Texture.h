#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>
#include "../typedefs.h"

class ImageData;

class Texture {
public:
	unsigned int id;
	int width;
	int height;
	Texture(unsigned int id, int width, int height);
	Texture(unsigned char* data, int width, int height, uint format);
	~Texture();

	void bind();
	void reload(unsigned char* data);

	static Texture* from(const ImageData* image);
};

#endif /* GRAPHICS_TEXTURE_H_ */
