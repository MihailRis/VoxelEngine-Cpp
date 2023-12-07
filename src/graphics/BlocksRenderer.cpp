#include "BlocksRenderer.h"

#include <glm/glm.hpp>

#include "Mesh.h"
#include "UVRegion.h"
#include "../constants.h"
#include "../content/Content.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../voxels/VoxelsVolume.h"
#include "../voxels/ChunksStorage.h"
#include "../lighting/Lightmap.h"
#include "../frontend/ContentGfxCache.h"

using glm::ivec3;
using glm::vec3;
using glm::vec4;

const uint BlocksRenderer::VERTEX_SIZE = 6;

BlocksRenderer::BlocksRenderer(size_t capacity,
	const Content* content,
	const ContentGfxCache* cache,
	const EngineSettings& settings)
	: content(content),
	vertexOffset(0),
	indexOffset(0),
	indexSize(0),
	capacity(capacity),
	cache(cache),
	settings(settings) {
	vertexBuffer = new float[capacity];
	indexBuffer = new int[capacity];
	voxelsBuffer = new VoxelsVolume(CHUNK_W + 2, CHUNK_H, CHUNK_D + 2);
	blockDefsCache = content->indices->getBlockDefs();
}

BlocksRenderer::~BlocksRenderer() {
	delete voxelsBuffer;
	delete[] vertexBuffer;
	delete[] indexBuffer;
}

/* Basic vertex add method */
void BlocksRenderer::vertex(const vec3& coord,
	float u, float v,
	const vec4& light) {
	vertexBuffer[vertexOffset++] = coord.x;
	vertexBuffer[vertexOffset++] = coord.y;
	vertexBuffer[vertexOffset++] = coord.z;

	vertexBuffer[vertexOffset++] = u;
	vertexBuffer[vertexOffset++] = v;

	union {
		float floating;
		uint32_t integer;
	} compressed;

	compressed.integer = (uint32_t(light.r * 255) & 0xff) << 24;
	compressed.integer |= (uint32_t(light.g * 255) & 0xff) << 16;
	compressed.integer |= (uint32_t(light.b * 255) & 0xff) << 8;
	compressed.integer |= (uint32_t(light.a * 255) & 0xff);

	vertexBuffer[vertexOffset++] = compressed.floating;
}

void BlocksRenderer::index(int a, int b, int c, int d, int e, int f) {
	indexBuffer[indexSize++] = indexOffset + a;
	indexBuffer[indexSize++] = indexOffset + b;
	indexBuffer[indexSize++] = indexOffset + c;
	indexBuffer[indexSize++] = indexOffset + d;
	indexBuffer[indexSize++] = indexOffset + e;
	indexBuffer[indexSize++] = indexOffset + f;
	indexOffset += 4;
}

/* Add face with precalculated lights */
void BlocksRenderer::face(const vec3& coord,
						  float w, float h,
						  const vec3& axisX,
						  const vec3& axisY,
						  const UVRegion& region,
						  const vec4(&lights)[4],
						  const vec4& tint) {
	if (vertexOffset + BlocksRenderer::VERTEX_SIZE * 4 > capacity) {
		overflow = true;
		return;
	}
	vertex(coord, region.u1, region.v1, lights[0] * tint);
	vertex(coord + axisX * w, region.u2, region.v1, lights[1] * tint);
	vertex(coord + axisX * w + axisY * h, region.u2, region.v2, lights[2] * tint);
	vertex(coord + axisY * h, region.u1, region.v2, lights[3] * tint);
	index(0, 1, 3, 1, 2, 3);
}

void BlocksRenderer::vertex(const ivec3& coord,
							float u, float v,
							const vec4& tint,
							const ivec3& axisX,
							const ivec3& axisY,
							const ivec3& axisZ) {
	vec4 light = pickSoftLight(coord+axisZ, axisX, axisY);
	vertex(coord, u, v, light * tint);
}

void BlocksRenderer::vertex(const vec3& coord,
							float u, float v,
							const vec4& tint,
							const ivec3& axisX,
							const ivec3& axisY,
							const ivec3& axisZ) {
	vec4 light = pickSoftLight(ivec3(coord.x, coord.y, coord.z)+axisZ, axisX, axisY);
	vertex(coord, u, v, light * tint);
}

void BlocksRenderer::face(const ivec3& coord,
						  const ivec3& axisX,
						  const ivec3& axisY,
						  const ivec3& axisZ,
						  const ivec3& laxisZ,
						  const UVRegion& region) {
	if (vertexOffset + BlocksRenderer::VERTEX_SIZE * 4 > capacity) {
		overflow = true;
		return;
	}

	const vec3 sunVector = vec3(0.411934f, 0.863868f, 0.279161f);
	float d = glm::dot(vec3(axisZ.x, axisZ.y, axisZ.z), sunVector);
	d = 0.7f +  d*0.3f;

	vec4 tint(d);

	vertex(coord, region.u1, region.v1, tint, axisX, axisY, laxisZ);
	vertex(coord + axisX, region.u2, region.v1, tint, axisX, axisY, laxisZ);
	vertex(coord + axisX + axisY, region.u2, region.v2, tint, axisX, axisY, laxisZ);
	vertex(coord + axisY, region.u1, region.v2, tint, axisX, axisY, laxisZ);
	index(0, 1, 2, 0, 2, 3);
}

void BlocksRenderer::face(const ivec3& coord_,
						  const ivec3& axisX,
						  const ivec3& axisY,
						  const ivec3& axisZ,
						  const ivec3& laxisZ,
						  const vec3& offset,
						  float width,
						  float height,
						  float depth,
						  const UVRegion& region) {
	if (vertexOffset + BlocksRenderer::VERTEX_SIZE * 4 > capacity) {
		overflow = true;
		return;
	}

	const vec3 X(axisX);
	const vec3 Y(axisY);
	const vec3 Z(axisZ);

	const vec3 sunVector = vec3(0.431934f, 0.863868f, 0.259161f);
	float d = glm::dot(Z, sunVector);
	d = 0.75f +  d*0.25f;

	vec4 tint(d);
	vec3 coord(vec3(coord_) + offset);

	vertex(coord + Z*depth, region.u1, region.v1, tint, axisX, axisY, laxisZ);
	vertex(coord + Z*depth + X*width, region.u2, region.v1, tint, axisX, axisY, laxisZ);
	vertex(coord + Z*depth + X*width + Y*height, region.u2, region.v2, tint, axisX, axisY, laxisZ);
	vertex(coord + Z*depth + Y*height, region.u1, region.v2, tint, axisX, axisY, laxisZ);
	index(0, 1, 2, 0, 2, 3);
}

void BlocksRenderer::blockCube(int x, int y, int z, const UVRegion(&texfaces)[6], ubyte group) {
	vec4 lights[]{ vec4(1.0f), vec4(1.0f), vec4(1.0f), vec4(1.0f) };
	if (isOpen(x, y, z + 1, group)) {
		face(vec3(x, y, z), 1, 1, vec3(1, 0, 0), vec3(0, 1, 0), texfaces[5], lights, vec4(1.0f));
	}
	if (isOpen(x, y, z - 1, group)) {
		face(vec3(x + 1, y, z - 1), 1, 1, vec3(-1, 0, 0), vec3(0, 1, 0), texfaces[4], lights, vec4(1.0f));
	}

	if (isOpen(x, y + 1, z, group)) {
		face(vec3(x, y + 1, z), 1, 1, vec3(1, 0, 0), vec3(0, 0, -1), texfaces[3], lights);
	}

	if (isOpen(x, y - 1, z, group)) {
		face(vec3(x, y, z - 1), 1, 1, vec3(1, 0, 0), vec3(0, 0, 1), texfaces[2], lights, vec4(1.0f));
	}

	if (isOpen(x - 1, y, z, group)) {
		face(vec3(x, y, z - 1), 1, 1, vec3(0, 0, 1), vec3(0, 1, 0), texfaces[0], lights, vec4(1.0f));
	}
	if (isOpen(x + 1, y, z, group)) {
		face(vec3(x + 1, y, z), 1, 1, vec3(0, 0, -1), vec3(0, 1, 0), texfaces[1], lights, vec4(1.0f));
	}
}

void BlocksRenderer::blockXSprite(int x, int y, int z,
								  const vec3& size,
								  const UVRegion& texface1,
								  const UVRegion& texface2,
								  float spread) {
	vec4 lights[]{
			pickSoftLight({x, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight({x + 1, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight({x + 1, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight({x, y + 1, z}, {1, 0, 0}, {0, 1, 0}) };

	int rand = ((x * z + y) ^ (z * y - x)) * (z + y);

	float xs = ((float)(char)rand / 512) * spread;
	float zs = ((float)(char)(rand >> 8) / 512) * spread;

	const float w = size.x / 1.41f;
	const float tint = 0.8f;
	face(vec3(x + xs + (1.0 - w) * 0.5f, y, z + zs - 1 + (1.0 - w) * 0.5f),
		w, size.y, vec3(1.0f, 0, 1.0f), vec3(0, 1, 0),
		texface1, lights, vec4(tint));
	face(vec3(x + xs - (1.0 - w) * 0.5f + 1, y, z + zs - (1.0 - w) * 0.5f),
		w, size.y, vec3(-1.0f, 0, -1.0f), vec3(0, 1, 0),
		texface1, lights, vec4(tint));

	face(vec3(x + xs + (1.0 - w) * 0.5f, y, z + zs - (1.0 - w) * 0.5f),
		w, size.y, vec3(1.0f, 0, -1.0f), vec3(0, 1, 0),
		texface2, lights, vec4(tint));
	face(vec3(x + xs - (1.0 - w) * 0.5f + 1, y, z + zs + (1.0 - w) * 0.5f - 1),
		w, size.y, vec3(-1.0f, 0, 1.0f), vec3(0, 1, 0),
		texface2, lights, vec4(tint));
}

/* AABB blocks render method (WIP) */
void BlocksRenderer::blockCubeShaded(const ivec3& icoord,
									 const vec3& offset,
									 const vec3& size,
									 const UVRegion(&texfaces)[6],
									 const Block* block, ubyte rotation) {

	ivec3 X(1, 0, 0);
	ivec3 Y(0, 1, 0);
	ivec3 Z(0, 0, 1);
	ivec3 loff(0);
	ivec3 coord = icoord;
	if (block->rotatable) {
		auto& rotations = block->rotations;
		auto& orient = rotations.variants[rotation];
		X = orient.axisX;
		Y = orient.axisY;
		Z = orient.axisZ;
		coord += orient.fix;
		loff -= orient.fix;
	}
	vec3 fX(X);
	vec3 fY(Y);
	vec3 fZ(Z);

	vec3 local = offset.x*vec3(X)+offset.y*vec3(Y)+offset.z*-fZ;

	face(coord, X, Y, Z, Z+loff, local-size.z*fZ, size.x, size.y, size.z, texfaces[5]); // north ;;;
	face(coord, -X, Y, -Z, Z-Z-X+loff, local-size.z*fZ+fX*size.x, size.x, size.y, 0.0f, texfaces[4]); // south ;;;

	face(coord+Y, X, -Z, Y, Y-Y+loff, local, size.x, size.z, 0.0f, texfaces[3]); // top ;;;
	face(coord+X, -X, -Z, -Y, -X-Y+loff, local+size.x*fX-fX, size.x, size.z, 0.0f, texfaces[2]); // bottom ;;;

	face(coord+X, -Z, Y, X, X-X+loff, local+size.x*fX-fX, size.z, size.y, 0.0f, texfaces[1]); // west ;;;
	face(coord+Y, -Z, -Y, -X, -X-Y+loff, local, size.z, size.y, 0.0f, texfaces[0]); // east ;;;
}

/* Fastest solid shaded blocks render method */
void BlocksRenderer::blockCubeShaded(int x, int y, int z,
									 const UVRegion(&texfaces)[6],
									 const Block* block,
									 ubyte states) {
	ubyte group = block->drawGroup;

	ivec3 X(1, 0, 0);
	ivec3 Y(0, 1, 0);
	ivec3 Z(0, 0, 1);
	ivec3 loff(0);
	ivec3 coord(x, y, z);
	if (block->rotatable) {
		auto& rotations = block->rotations;
		auto& orient = rotations.variants[states & BLOCK_ROT_MASK];
		X = orient.axisX;
		Y = orient.axisY;
		Z = orient.axisZ;
		coord += orient.fix;
		loff -= orient.fix;
	}
	
	if (isOpen(x+Z.x, y+Z.y, z+Z.z, group)) {
		face(coord, X, Y, Z, Z+loff, texfaces[5]);
	}
	if (isOpen(x-Z.x, y-Z.y, z-Z.z, group)) {
		face(coord+X-Z, -X, Y, -Z, Z-Z-X+loff, texfaces[4]);
	}
	if (isOpen(x+Y.x, y+Y.y, z+Y.z, group)) {
		face(coord+Y, X, -Z, Y, Y-Y+loff, texfaces[3]);
	}
	if (isOpen(x-Y.x, y-Y.y, z-Y.z, group)) {
		face(coord-Z, X, Z, -Y, -Y+Z+loff, texfaces[2]);
	}
	if (isOpen(x+X.x, y+X.y, z+X.z, group)) {
		face(coord+X, -Z, Y, X, X-X+loff, texfaces[1]);
	}
	if (isOpen(x-X.x, y-X.y, z-X.z, group)) {
		face(coord-Z, Z, Y, -X, -X+Z+loff, texfaces[0]);
	}
}

// Does block allow to see other blocks sides (is it transparent)
bool BlocksRenderer::isOpen(int x, int y, int z, ubyte group) const {
	blockid_t id = voxelsBuffer->pickBlockId(chunk->x * CHUNK_W + x,
											 y,
											 chunk->z * CHUNK_D + z);
	if (id == BLOCK_VOID)
		return false;
	const Block& block = *blockDefsCache[id];
	if ((block.drawGroup != group && block.lightPassing) || !block.rt.solid) {
		return true;
	}
	return !id;
}

bool BlocksRenderer::isOpenForLight(int x, int y, int z) const {
	blockid_t id = voxelsBuffer->pickBlockId(chunk->x * CHUNK_W + x,
											 y,
											 chunk->z * CHUNK_D + z);
	if (id == BLOCK_VOID)
		return false;
	const Block& block = *blockDefsCache[id];
	if (block.lightPassing) {
		return true;
	}
	return !id;
}

vec4 BlocksRenderer::pickLight(int x, int y, int z) const {
	if (isOpenForLight(x, y, z)) {
		light_t light = voxelsBuffer->pickLight(chunk->x * CHUNK_W + x,
												y,
												chunk->z * CHUNK_D + z);
		return vec4(Lightmap::extract(light, 0) / 15.0f,
			Lightmap::extract(light, 1) / 15.0f,
			Lightmap::extract(light, 2) / 15.0f,
			Lightmap::extract(light, 3) / 15.0f);
	}
	else {
		return vec4(0.0f);
	}
}

vec4 BlocksRenderer::pickLight(const ivec3& coord) const {
	return pickLight(coord.x, coord.y, coord.z);
}

vec4 BlocksRenderer::pickSoftLight(const ivec3& coord, 
								   const ivec3& right,
								   const ivec3& up) const {
	return (
		pickLight(coord) +
		pickLight(coord - right) +
		pickLight(coord - right - up) +
		pickLight(coord - up)) * 0.25f;
}

vec4 BlocksRenderer::pickSoftLight(float x, float y, float z, 
								  const ivec3& right,
								  const ivec3& up) const {
	return pickSoftLight({int(round(x)), int(round(y)), int(round(z))}, right, up);
}

void BlocksRenderer::render(const voxel* voxels) {
	int begin = chunk->bottom * (CHUNK_W * CHUNK_D);
	int end = chunk->top * (CHUNK_W * CHUNK_D);
	for (const auto drawGroup : *content->drawGroups) {
		for (int i = begin; i < end; i++) {
			const voxel& vox = voxels[i];
			blockid_t id = vox.id;
			const Block& def = *blockDefsCache[id];
			if (!id || def.drawGroup != drawGroup)
				continue;
			const UVRegion texfaces[6]{ cache->getRegion(id, 0),
										cache->getRegion(id, 1),
										cache->getRegion(id, 2),
										cache->getRegion(id, 3),
										cache->getRegion(id, 4),
										cache->getRegion(id, 5)};
			int x = i % CHUNK_W;
			int y = i / (CHUNK_D * CHUNK_W);
			int z = (i / CHUNK_D) % CHUNK_W;
			switch (def.model) {
			case BlockModel::block:
				if (def.rt.emissive) {
					blockCube(x, y, z, texfaces, def.drawGroup);
				}
				else {
					blockCubeShaded(x, y, z, texfaces, &def, vox.states);
				}
				break;
			case BlockModel::xsprite: {
				blockXSprite(x, y, z, vec3(1.0f),
							 texfaces[FACE_MX], texfaces[FACE_MZ], 1.0f);
				break;
			}
			case BlockModel::aabb: {
				AABB hitbox = def.hitbox;
				hitbox.a = vec3(1.0f)-hitbox.a;
				hitbox.b = vec3(1.0f)-hitbox.b;

				vec3 size = hitbox.size();
				vec3 off = hitbox.min();
				blockCubeShaded(ivec3(x,y,z), off, size, texfaces, &def, vox.rotation());
				break;
			}
			default:
				break;
			}
			if (overflow)
				return;
		}
	}
}

#ifdef USE_VULKAN
vulkan::Mesh<Vertex3D>* BlocksRenderer::renderVulkanMesh(const Chunk* chunk,
	const ChunksStorage* chunks) {
	this->chunk = chunk;
	this->chunk = chunk;
	voxelsBuffer->setPosition(chunk->x * CHUNK_W - 1, 0, chunk->z * CHUNK_D - 1);
	chunks->getVoxels(voxelsBuffer, settings.graphics.backlight);
	overflow = false;
	vertexOffset = 0;
	indexOffset = indexSize = 0;
	const voxel* voxels = chunk->voxels;
	render(voxels);

	const Vertex3D *vertices = reinterpret_cast<Vertex3D *>(vertexBuffer);
	auto *mesh = new vulkan::Mesh(vertices, vertexOffset / VERTEX_SIZE, indexBuffer, indexSize);
	return mesh;
}
#endif

Mesh* BlocksRenderer::render(const Chunk* chunk, const ChunksStorage* chunks) {
	this->chunk = chunk;
	voxelsBuffer->setPosition(chunk->x * CHUNK_W - 1, 0, chunk->z * CHUNK_D - 1);
	chunks->getVoxels(voxelsBuffer, settings.graphics.backlight);
	overflow = false;
	vertexOffset = 0;
	indexOffset = indexSize = 0;
	const voxel* voxels = chunk->voxels;
	render(voxels);

	const vattr attrs[]{ {3}, {2}, {1}, {0} };
	size_t vcount = vertexOffset / BlocksRenderer::VERTEX_SIZE;
	Mesh* mesh = new Mesh(vertexBuffer, vcount, indexBuffer, indexSize, attrs);
	return mesh;
}

VoxelsVolume* BlocksRenderer::getVoxelsBuffer() const {
	return voxelsBuffer;
}
