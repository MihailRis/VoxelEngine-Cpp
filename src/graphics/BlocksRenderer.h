#ifndef GRAPHICS_BLOCKS_RENDERER_H
#define GRAPHICS_BLOCKS_RENDERER_H

#include <stdlib.h>
#include <glm/glm.hpp>
#include "UVRegion.h"
#include "../typedefs.h"
#include "../voxels/voxel.h"

class Mesh;
class Block;
class Chunk;
class Chunks;
class VoxelsVolume;
class ChunksStorage;

class BlocksRenderer {
	float* buffer;
	size_t offset;
	size_t capacity;

	bool overflow = false;

	const Chunk* chunk = nullptr;
	VoxelsVolume* voxelsBuffer;
	
	void vertex(const glm::vec3& coord, float u, float v, const glm::vec4& light);

	void face(const glm::vec3& coord, float w, float h,
			const glm::vec3& axisX,
			const glm::vec3& axisY,
			const UVRegion& region,
			const glm::vec4 (&lights)[4],
			const glm::vec4& tint);

	void face(const glm::vec3& coord, float w, float h,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
		const UVRegion& region,
		const glm::vec4 (&lights)[4],
		const glm::vec4& tint,
		bool rotated);

	void face(const glm::vec3& coord, float w, float h,
			const glm::vec3& axisX,
			const glm::vec3& axisY,
			const UVRegion& region,
			const glm::vec4 (&lights)[4]) {
		face(coord, w, h, axisX, axisY, region, lights, glm::vec4(1.0f));
	}

	void cube(const glm::vec3& coord, const glm::vec3& size, const UVRegion (&faces)[6]);
	void blockCube(int x, int y, int z, const glm::vec3& size, const UVRegion (&faces)[6], ubyte group);
	void blockCubeShaded(int x, int y, int z, const glm::vec3& size, const UVRegion (&faces)[6], const Block* block, ubyte states);
	void blockXSprite(int x, int y, int z, const glm::vec3& size, const UVRegion& face1, const UVRegion& face2, float spread);

	bool isOpenForLight(int x, int y, int z) const;
	bool isOpen(int x, int y, int z, ubyte group) const;

	glm::vec4 pickLight(int x, int y, int z) const;
	glm::vec4 pickSoftLight(int x, int y, int z, const glm::ivec3& right, const glm::ivec3& up) const;
	void render(const voxel* voxels, int atlas_size);
public:
	BlocksRenderer(size_t capacity);
	virtual ~BlocksRenderer();

	Mesh* render(const Chunk* chunk, int atlas_size, const ChunksStorage* chunks);
	VoxelsVolume* getVoxelsBuffer() const;
};

#endif // GRAPHICS_BLOCKS_RENDERER_H