#ifndef GRAPHICS_BLOCKS_RENDERER_H
#define GRAPHICS_BLOCKS_RENDERER_H

#include <stdlib.h>
#include <vector>
#include <glm/glm.hpp>
#include "../../graphics/UVRegion.h"
#include "../../voxels/voxel.h"
#include "../../typedefs.h"
#include "../../settings.h"

class Content;
class Mesh;
class Block;
class Chunk;
class Chunks;
class VoxelsVolume;
class ChunksStorage;
class ContentGfxCache;

class BlocksRenderer {
    static const glm::vec3 SUN_VECTOR;
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

	void vertex(const glm::vec3& coord, float u, float v, 
				const glm::vec4& brightness,
				const glm::vec3& axisX,
				const glm::vec3& axisY,
				const glm::vec3& axisZ);

	void face(const glm::vec3& coord, float w, float h, float d,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
        const glm::vec3& axisZ,
		const UVRegion& region,
		const glm::vec4(&lights)[4],
		const glm::vec4& tint);
	
	void face(const glm::vec3& coord,
		const glm::vec3& axisX,
		const glm::vec3& axisY,
		const glm::vec3& axisZ,
		const UVRegion& region,
        bool lights);

	void tetragonicFace(const glm::vec3& coord,
		const glm::vec3& p1, const glm::vec3& p2,
		const glm::vec3& p3, const glm::vec3& p4,
		const glm::vec3& X,
		const glm::vec3& Y,
		const glm::vec3& Z,
		const UVRegion& texreg,
		bool lights);
	
	void blockCube(int x, int y, int z, const UVRegion(&faces)[6], const Block* block, ubyte states, bool lights);
	void blockAABB(const glm::ivec3& coord,
                    const UVRegion(&faces)[6], 
                    const Block* block, 
                    ubyte rotation,
                    bool lights);
	void blockXSprite(int x, int y, int z, const glm::vec3& size, const UVRegion& face1, const UVRegion& face2, float spread);
	void blockCustomModel(const glm::ivec3& icoord,
		const Block* block, ubyte rotation,
		bool lights);

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

    void build(const Chunk* chunk, const ChunksStorage* chunks);
	Mesh* render(const Chunk* chunk, const ChunksStorage* chunks);
    Mesh* createMesh();
	VoxelsVolume* getVoxelsBuffer() const;
};

#endif // GRAPHICS_BLOCKS_RENDERER_H