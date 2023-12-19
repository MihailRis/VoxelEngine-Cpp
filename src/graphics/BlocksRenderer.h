#ifndef GRAPHICS_BLOCKS_RENDERER_H
#define GRAPHICS_BLOCKS_RENDERER_H

#include <stdlib.h>
#include <glm/glm.hpp>
#include "UVRegion.h"
#include "../typedefs.h"
#include "../voxels/voxel.h"
#include "../settings.h"

class Content;
class Mesh;
class Block;
class Chunk;
class Chunks;
class VoxelsVolume;
class ChunksStorage;
class ContentGfxCache;

class BlocksRenderer {
	static const uint VERTEX_SIZE;
	const Content* const content;
	float* vertexBuffer;
	int* indexBuffer;
	size_t vertexOffset;
	size_t indexOffset, indexSize;
	size_t capacity;

	bool overflow = false;

	const Chunk* chunk = nullptr;
	VoxelsVolume* voxelsBuffer;

	const Block* const* blockDefsCache;
	const ContentGfxCache* const cache;
	const EngineSettings& settings;

	void vertex(const glm::vec3& coord, float u, float v, const glm::vec4& light);
	void index(int a, int b, int c, int d, int e, int f);

	void vertex(const glm::ivec3& coord, float u, float v, 
				const glm::vec4& brightness,
				const glm::ivec3& axisX,
				const glm::ivec3& axisY,
				const glm::ivec3& axisZ);

	void vertex(const glm::vec3& coord, 
                float u, float v, 
				const glm::vec4& brightness,
				const glm::ivec3& axisX,
				const glm::ivec3& axisY,
				const glm::ivec3& axisZ);

	void face(const glm::vec3& coord, float w, float h,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
		const UVRegion& region,
		const glm::vec4(&lights)[4],
		const glm::vec4& tint);
	
	void face(const glm::ivec3& coord,
		const glm::ivec3& axisX,
		const glm::ivec3& axisY,
		const glm::ivec3& axisZ,
		const glm::ivec3& laxisZ,
		const UVRegion& region);

	void face(const glm::vec3& coord,
		const glm::ivec3& axisX,
		const glm::ivec3& axisY,
		const glm::ivec3& axisZ,
		const glm::ivec3& laxisZ,
		float width,
		float height,
		float depth,
		const UVRegion& region,
        bool lights);

	void face(const glm::vec3& coord, float w, float h,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
		const UVRegion& region,
		const glm::vec4(&lights)[4]) {
		face(coord, w, h, axisX, axisY, region, lights, glm::vec4(1.0f));
	}

	void blockCube(int x, int y, int z, const UVRegion(&faces)[6], ubyte group);
	
	void blockCubeShaded(int x, int y, int z, const UVRegion(&faces)[6], const Block* block, ubyte states);
	void blockAABB(const glm::ivec3& coord,
                    const UVRegion(&faces)[6], 
                    const Block* block, 
                    ubyte rotation,
                    bool lights);
	void blockXSprite(int x, int y, int z, const glm::vec3& size, const UVRegion& face1, const UVRegion& face2, float spread);

	bool isOpenForLight(int x, int y, int z) const;
	bool isOpen(int x, int y, int z, ubyte group) const;

	glm::vec4 pickLight(int x, int y, int z) const;
	glm::vec4 pickLight(const glm::ivec3& coord) const;
	glm::vec4 pickSoftLight(const glm::ivec3& coord, const glm::ivec3& right, const glm::ivec3& up) const;
	glm::vec4 pickSoftLight(float x, float y, float z, const glm::ivec3& right, const glm::ivec3& up) const;
	void render(const voxel* voxels);
public:
	BlocksRenderer(size_t capacity, const Content* content, const ContentGfxCache* cache, const EngineSettings& settings);
	virtual ~BlocksRenderer();

	Mesh* render(const Chunk* chunk, const ChunksStorage* chunks);
	VoxelsVolume* getVoxelsBuffer() const;
};

#endif // GRAPHICS_BLOCKS_RENDERER_H