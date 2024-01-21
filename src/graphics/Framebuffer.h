#ifndef SRC_GRAPHICS_FRAMEBUFFER_H_
#define SRC_GRAPHICS_FRAMEBUFFER_H_

#include "../typedefs.h"

class Texture;

class Framebuffer {
	uint fbo;
	uint depth;
public:
	uint width;
	uint height;
	Texture* texture;
	Framebuffer(uint width, uint height, bool alpha=false);
	~Framebuffer();

	void bind();
	void unbind();
};

#endif /* SRC_GRAPHICS_FRAMEBUFFER_H_ */
