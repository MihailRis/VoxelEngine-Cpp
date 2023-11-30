#ifndef GRAPHICS_BATCH3D_H_
#define GRAPHICS_BATCH3D_H_

#include <stdlib.h>
#include <glm/glm.hpp>

class Mesh;
class Texture;

class Batch3D {
	float* buffer;
	size_t capacity;
	size_t offset;
	glm::vec4 color;
	Mesh* mesh;
	size_t index;

	Texture* blank;
	Texture* _texture;

	void vertex(float x, float y, float z,
				float u, float v,
				float r, float g, float b, float a);
	void vertex(glm::vec3 point, glm::vec2 uvpoint,
				float r, float g, float b, float a);

public:
	Batch3D(size_t capacity);
	~Batch3D();

	void begin();
	void texture(Texture* texture);
	void sprite(glm::vec3 pos, glm::vec3 up, glm::vec3 right, float w, float h, 
				int atlasRes, int index, glm::vec4 tint);
	void sprite(glm::vec3 pos, glm::vec3 up, glm::vec3 right, float w, float h);
	void render();
};

#endif /* GRAPHICS_BATCH3D_H_ */
