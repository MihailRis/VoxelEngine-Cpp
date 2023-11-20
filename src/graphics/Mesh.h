#ifndef GRAPHICS_MESH_H_
#define GRAPHICS_MESH_H_

#include <stdlib.h>
#include "../typedefs.h"

struct vattr {
	ubyte size;
};

class Mesh {
	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;
	size_t vertices;
	size_t indices;
	size_t vertexSize;
public:
	Mesh(const float* vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices, const vattr* attrs);
	Mesh(const float* vertexBuffer, size_t vertices, const vattr* attrs) :
		Mesh(vertexBuffer, vertices, nullptr, 0, attrs) {};
	~Mesh();

	void reload(const float* vertexBuffer, size_t vertices, const int* indexBuffer = nullptr, size_t indices = 0);
	void draw(unsigned int primitive);
	void draw();

	static int meshesCount;
};

#endif /* GRAPHICS_MESH_H_ */
