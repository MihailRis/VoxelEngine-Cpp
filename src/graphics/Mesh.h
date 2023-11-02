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
	size_t vertices;
	size_t vertexSize;
public:
	Mesh(const float* buffer, size_t vertices, const vattr* attrs);
	~Mesh();

	void reload(const float* buffer, size_t vertices);
	void draw(unsigned int primitive);
	void draw();

	static int meshesCount;
};

#endif /* GRAPHICS_MESH_H_ */
