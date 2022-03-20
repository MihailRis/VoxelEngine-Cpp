#ifndef SRC_GRAPHICS_BATCH2D_H_
#define SRC_GRAPHICS_BATCH2D_H_

#include <stdlib.h>
#include <glm/glm.hpp>

class Mesh;
class Texture;

class Batch2D {
	float* buffer;
	size_t capacity;
	size_t offset;
	glm::vec4 color;
	Mesh* mesh;
	size_t index;

	Texture* blank;
	Texture* _texture;

	void vertex(float x, float y,
			float u, float v,
			float r, float g, float b, float a);
public:
	Batch2D(size_t capacity);
	~Batch2D();

	void begin();
	void texture(Texture* texture);
	void rect(float x, float y, float w, float h);
	void rect(float x, float y, float w, float h,
						float u, float v, float tx, float ty,
						float r, float g, float b, float a);
	void render();
};

#endif /* SRC_GRAPHICS_BATCH2D_H_ */
