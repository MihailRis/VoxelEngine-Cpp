#ifndef GRAPHICS_TEXTURE_H_
#define GRAPHICS_TEXTURE_H_

#include <string>
#include "../typedefs.h"

class ImageData;

class Texture {
public:
	uint id;
	int width;
	int height;
	Texture(uint id, int width, int height);
	Texture(u_char8* data, int width, int height, uint format);
	~Texture();

	void bind();
	void reload(u_char8* data);

	static Texture* from(const ImageData* image);
};

#endif /* GRAPHICS_TEXTURE_H_ */
