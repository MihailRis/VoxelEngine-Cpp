#ifndef SRC_GRAPHICS_FRAMEBUFFER_H_
#define SRC_GRAPHICS_FRAMEBUFFER_H_

class Texture;

class Framebuffer {
	unsigned int fbo;
	unsigned int depth;
public:
	int width;
	int height;
	Texture* texture;
	Framebuffer(int width, int height);
	~Framebuffer();

	void bind();
	void unbind();
};

#endif /* SRC_GRAPHICS_FRAMEBUFFER_H_ */
