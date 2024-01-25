#ifndef GRAPHICS_BATCH3D_H_
#define GRAPHICS_BATCH3D_H_

#include <stdlib.h>
#include <glm/glm.hpp>
#include "UVRegion.h"
#include "../typedefs.h"

class Mesh;
class Texture;

class Batch3D {
	float* buffer;
	size_t capacity;
	size_t offset;
	Mesh* mesh;
	size_t index;

	Texture* blank;
	Texture* _texture;

	void vertex(float x, float y, float z,
				float u, float v,
				float r, float g, float b, float a);
	void vertex(glm::vec3 coord,
			float u, float v,
			float r, float g, float b, float a);
	void vertex(glm::vec3 point, glm::vec2 uvpoint,
				float r, float g, float b, float a);

	void face(const glm::vec3& coord, float w, float h,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
		const UVRegion& region,
		const glm::vec4& tint);

public:
	Batch3D(size_t capacity);
	~Batch3D();

	void begin();
	void texture(Texture* texture);
	void sprite(glm::vec3 pos, glm::vec3 up, glm::vec3 right, float w, float h, const UVRegion& uv, glm::vec4 tint);
	void xSprite(float w, float h, const UVRegion& uv, const glm::vec4 tint, bool shading=true);
	void blockCube(const glm::vec3 size, const UVRegion(&texfaces)[6], const glm::vec4 tint, bool shading=true);
    void point(glm::vec3 pos, glm::vec4 tint);
	void flush();
    void flushPoints();
};

#endif /* GRAPHICS_BATCH3D_H_ */
