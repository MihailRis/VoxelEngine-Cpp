#ifndef GRAPHICS_VOXELRENDERER_H_
#define GRAPHICS_VOXELRENDERER_H_

#include <stdlib.h>
#include <vector>

class Mesh;
class Chunk;

#define CHUNK_VERTEX_SIZE (3 + 2 + 4)

class VoxelRenderer {
public:
	std::vector<float> buffer;
	VoxelRenderer();
	~VoxelRenderer();

	const float* render(Chunk* chunk, const Chunk** chunks, size_t& size);
};

#endif /* GRAPHICS_VOXELRENDERER_H_ */
