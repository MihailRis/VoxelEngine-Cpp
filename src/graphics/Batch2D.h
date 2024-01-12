#ifndef SRC_GRAPHICS_BATCH2D_H_
#define SRC_GRAPHICS_BATCH2D_H_

#include <stdlib.h>
#include <glm/glm.hpp>

#include "UVRegion.h"

class Mesh;
class Texture;
class Sprite;

class Batch2D {
	float* buffer;
	size_t capacity;
	size_t offset;
	Mesh* mesh;
	size_t index;

	Texture* blank;
	Texture* _texture;

	void vertex(float x, float y,
			float u, float v,
			float r, float g, float b, float a);
	void vertex(glm::vec2 point,
			glm::vec2 uvpoint,
			float r, float g, float b, float a);

public:
	glm::vec4 color;

	Batch2D(size_t capacity);
	~Batch2D();

	void begin();
	void texture(Texture* texture);
	void sprite(float x, float y, float w, float h, const UVRegion& region, glm::vec4 tint);
	void sprite(Sprite* sprite);
	void sprite(float x, float y, float w, float h, int atlasRes, int index, glm::vec4 tint);
	void point(float x, float y, float r, float g, float b, float a);
	void line(float x1, float y1, float x2, float y2, float r, float g, float b, float a);
	void rect(float x, float y,
			float w, float h,
			float ox, float oy,
			float angle, UVRegion region,
			bool flippedX, bool flippedY,
			glm::vec4 tint);

	void rect(float x, float y, float w, float h);
	void rect(float x, float y, float w, float h,
			float u, float v, float tx, float ty,
			float r, float g, float b, float a);

	void rect(float x, float y, float w, float h,
			float r0, float g0, float b0,
			float r1, float g1, float b1,
			float r2, float g2, float b2,
			float r3, float g3, float b3,
			float r4, float g4, float b4, int sh);
	void render(unsigned int gl_primitive);
	void render();

	void lineWidth(float width);
};

#endif /* SRC_GRAPHICS_BATCH2D_H_ */
