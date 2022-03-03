#ifndef SRC_GRAPHICS_BATCH2D_H_
#define SRC_GRAPHICS_BATCH2D_H_

#include <stdlib.h>
#include <glm/glm.hpp>

class Mesh;

class Batch2D {
	float* buffer;
	size_t capacity;
	size_t offset;
	glm::vec4 color;
	Mesh* mesh;

	void vertex(float x, float y,
			float u, float v,
			float r, float g, float b, float a);
public:
	Batch2D(size_t capacity);
	~Batch2D();

	void rect(float x, float y, float w, float h);
	void render();
};

#endif /* SRC_GRAPHICS_BATCH2D_H_ */
