#ifndef SRC_GRAPHICS_FRAMEBUFFER_H_
#define SRC_GRAPHICS_FRAMEBUFFER_H_

#include "../typedefs.h"

#include <memory>

class Texture;

class Framebuffer {
	uint fbo;
	uint depth;
	uint width;
	uint height;
	std::unique_ptr<Texture> texture;
public:
	Framebuffer(uint fbo, uint depth, std::unique_ptr<Texture> texture);
	Framebuffer(uint width, uint height, bool alpha=false);
	~Framebuffer();

	void bind();
	void unbind();

	Texture* getTexture() const;
	uint getWidth() const;
	uint getHeight() const;
};

#endif /* SRC_GRAPHICS_FRAMEBUFFER_H_ */
