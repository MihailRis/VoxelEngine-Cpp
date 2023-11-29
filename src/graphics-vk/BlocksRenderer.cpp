//
// Created by chelovek on 11/29/23.
//

#include "BlocksRenderer.h"

#include "../voxels/VoxelsVolume.h"
#include "../graphics/UVRegion.h"
#include "../lighting/Lightmap.h"
#include "../voxels/Block.h"
#include "../voxels/Chunk.h"
#include "../voxels/ChunksStorage.h"
#include "../frontend/ContentGfxCache.h"
#include "../settings.h"

namespace vulkan {
    void BlocksRenderer::vertex(const glm::vec3& coord, float u, float v, const glm::vec4& light) {
        union {
            float floating;
            uint32_t integer;
        } compressed = {};

        compressed.integer = (uint32_t(light.r * 255) & 0xff) << 24;
        compressed.integer |= (uint32_t(light.g * 255) & 0xff) << 16;
        compressed.integer |= (uint32_t(light.b * 255) & 0xff) << 8;
        compressed.integer |= (uint32_t(light.a * 255) & 0xff);

        m_vertexBuffer[m_vertexOffset++] = VertexMain{coord, {u, v}, compressed.floating};
    }

    void BlocksRenderer::index(int a, int b, int c, int d, int e, int f) {
        m_indexBuffer[m_indexSize++] = static_cast<int>(m_indexOffset + a);
        m_indexBuffer[m_indexSize++] = static_cast<int>(m_indexOffset + b);
        m_indexBuffer[m_indexSize++] = static_cast<int>(m_indexOffset + c);
        m_indexBuffer[m_indexSize++] = static_cast<int>(m_indexOffset + d);
        m_indexBuffer[m_indexSize++] = static_cast<int>(m_indexOffset + e);
        m_indexBuffer[m_indexSize++] = static_cast<int>(m_indexOffset + f);
        m_indexOffset += 4;
    }

    void BlocksRenderer::face(const glm::vec3& coord, float w, float h,
        const glm::vec3& axisX,
        const glm::vec3& axisY,
        const UVRegion& region,
        const glm::vec4(& lights)[4],
        const glm::vec4& tint) {
        if (m_vertexOffset > m_capacity) {
            overflow = true;
            return;
        }
        vertex(coord, region.u1, region.v1, lights[0] * tint);
        vertex(coord + axisX * w, region.u2, region.v1, lights[1] * tint);
        vertex(coord + axisX * w + axisY * h, region.u2, region.v2, lights[2] * tint);
        vertex(coord + axisY * h, region.u1, region.v2, lights[3] * tint);
        index(0, 1, 3, 1, 2, 3);
    }

    void BlocksRenderer::face(const glm::vec3& coord, float w, float h,
        const glm::vec3& axisX,
        const glm::vec3& axisY,
        const UVRegion& region,
        const glm::vec4(& lights)[4],
        const glm::vec4& tint, bool rotated) {
        if (m_vertexOffset > m_capacity) {
            overflow = true;
            return;
        }
        if (rotated) {
            vertex(coord, region.u2, region.v1, lights[0] * tint);
            vertex(coord + axisX * w, region.u2, region.v2, lights[1] * tint);
            vertex(coord + axisX * w + axisY * h, region.u1, region.v2, lights[2] * tint);
            vertex(coord + axisY * h, region.u1, region.v1, lights[3] * tint);
            index(0, 1, 2, 0, 2, 3);
        }
        else {
            vertex(coord, region.u1, region.v1, lights[0] * tint);
            vertex(coord + axisX * w, region.u2, region.v1, lights[1] * tint);
            vertex(coord + axisX * w + axisY * h, region.u2, region.v2, lights[2] * tint);
            vertex(coord + axisY * h, region.u1, region.v2, lights[3] * tint);
            index(0, 1, 2, 0, 2, 3);
        }
    }

    inline glm::vec4 do_tint(float value) {
        return glm::vec4(value);
    }

    void BlocksRenderer::cube(const glm::vec3& coord, const glm::vec3& size, const UVRegion(& faces)[6]) {
        glm::vec4 lights[]{ glm::vec4(),glm::vec4(),glm::vec4(),glm::vec4() };

        face(coord, size.x, size.y, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), faces[0], lights);
        face(coord + glm::vec3(size.x, 0, -size.z), size.x, size.y, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), faces[1], lights);

        face(coord + glm::vec3(0, size.y, 0), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), faces[2], lights);
        face(coord + glm::vec3(0, 0, -size.z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), faces[3], lights);

        face(coord + glm::vec3(0, 0, -size.z), size.z, size.y, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), faces[4], lights);
        face(coord + glm::vec3(size.x, 0, 0), size.z, size.y, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), faces[5], lights);
    }

    void BlocksRenderer::blockCube(int x, int y, int z, const glm::vec3& size, const UVRegion(& faces)[6],
        ubyte group) {
        glm::vec4 lights[]{ glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f), glm::vec4(1.0f) };
        if (isOpen(x, y, z + 1, group)) {
            face(glm::vec3(x, y, z), size.x, size.y, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), faces[5], lights, do_tint(1.0));
        }
        if (isOpen(x, y, z - 1, group)) {
            face(glm::vec3(x + size.x, y, z - size.z), size.x, size.y, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), faces[4], lights, glm::vec4(1.0f));
        }

        if (isOpen(x, y + 1, z, group)) {
            face(glm::vec3(x, y + size.y, z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), faces[3], lights);
        }

        if (isOpen(x, y - 1, z, group)) {
            face(glm::vec3(x, y, z - size.z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), faces[2], lights, glm::vec4(1.0f));
        }

        if (isOpen(x - 1, y, z, group)) {
            face(glm::vec3(x, y, z - size.z), size.z, size.y, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), faces[0], lights, glm::vec4(1.0f));
        }
        if (isOpen(x + 1, y, z, group)) {
            face(glm::vec3(x + size.x, y, z), size.z, size.y, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), faces[1], lights, glm::vec4(1.0f));
        }
    }

    void BlocksRenderer::blockCubeShaded(int x, int y, int z, const glm::vec3& size, const UVRegion(& faces)[6],
        const Block* block, ubyte states) {
        ubyte group = block->drawGroup;
        UVRegion texfaces[6];
        int rot = 0;

        for (int i = 0; i < 6; i++) {
	        texfaces[i] = faces[i];
        }

        if (block->rotatable) {
	        if (states == BLOCK_DIR_X) {
		        rot = 1;
		        texfaces[0] = faces[2];
		        texfaces[1] = faces[3];
		        texfaces[2] = faces[0];
		        texfaces[3] = faces[1];
	        }
	        else if (states == BLOCK_DIR_Y) {
		        rot = 2;
	        }
	        else if (states == BLOCK_DIR_Z) {
		        rot = 3;
		        texfaces[2] = faces[4];
		        texfaces[3] = faces[5];
		        texfaces[4] = faces[2];
		        texfaces[5] = faces[3];
	        }
        }
        if (isOpen(x, y, z + 1, group)) {
	        glm::vec4 lights[]{
		        pickSoftLight(x, y, z + 1, {1, 0, 0}, {0, 1, 0}),
		        pickSoftLight(x + 1, y, z + 1, {1, 0, 0}, {0, 1, 0}),
		        pickSoftLight(x + 1, y + 1, z + 1, {1, 0, 0}, {0, 1, 0}),
		        pickSoftLight(x, y + 1, z + 1, {1, 0, 0}, {0, 1, 0})
	        };
	        face(glm::vec3(x, y, z), size.x, size.y, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), texfaces[5], lights, do_tint(0.9f),
	             rot == 1);
        }
        if (isOpen(x, y, z - 1, group)) {
	        glm::vec4 lights[]{
		        pickSoftLight(x, y, z - 1, {-1, 0, 0}, {0, 1, 0}),
		        pickSoftLight(x - 1, y, z - 1, {-1, 0, 0}, {0, 1, 0}),
		        pickSoftLight(x - 1, y + 1, z - 1, {-1, 0, 0}, {0, 1, 0}),
		        pickSoftLight(x, y + 1, z - 1, {-1, 0, 0}, {0, 1, 0})
	        };
	        face(glm::vec3(x + size.x, y, z - size.z), size.x, size.y, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), texfaces[4], lights,
	             do_tint(0.75f), rot == 1);
        }

        if (isOpen(x, y + 1, z, group)) {
	        glm::vec4 lights[]{
		        pickSoftLight(x, y + 1, z + 1, {1, 0, 0}, {0, 0, 1}),
		        pickSoftLight(x + 1, y + 1, z + 1, {1, 0, 0}, {0, 0, 1}),
		        pickSoftLight(x + 1, y + 1, z, {1, 0, 0}, {0, 0, 1}),
		        pickSoftLight(x, y + 1, z, {1, 0, 0}, {0, 0, 1})
	        };

	        face(glm::vec3(x, y + size.y, z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), texfaces[3], lights, glm::vec4(1.0f),
	             rot == 1);
        }

        if (isOpen(x, y - 1, z, group)) {
	        glm::vec4 lights[]{
		        pickSoftLight(x, y - 1, z - 1, {1, 0, 0}, {0, 0, -1}),
		        pickSoftLight(x + 1, y - 1, z - 1, {1, 0, 0}, {0, 0, -1}),
		        pickSoftLight(x + 1, y - 1, z, {1, 0, 0}, {0, 0, -1}),
		        pickSoftLight(x, y - 1, z, {1, 0, 0}, {0, 0, -1})
	        };
	        face(glm::vec3(x, y, z - size.z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), texfaces[2], lights,
	             do_tint(0.6f), rot == 1);
        }

        if (isOpen(x - 1, y, z, group)) {
	        glm::vec4 lights[]{
		        pickSoftLight(x - 1, y, z - 1, {0, 0, -1}, {0, 1, 0}),
		        pickSoftLight(x - 1, y, z, {0, 0, -1}, {0, 1, 0}),
		        pickSoftLight(x - 1, y + 1, z, {0, 0, -1}, {0, 1, 0}),
		        pickSoftLight(x - 1, y + 1, z - 1, {0, 0, -1}, {0, 1, 0})
	        };
	        face(glm::vec3(x, y, z - size.z), size.z, size.y, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), texfaces[0], lights,
	             do_tint(0.7f), rot == 3);
        }
        if (isOpen(x + 1, y, z, group)) {
	        glm::vec4 lights[]{
		        pickSoftLight(x + 1, y, z, {0, 0, -1}, {0, 1, 0}),
		        pickSoftLight(x + 1, y, z - 1, {0, 0, -1}, {0, 1, 0}),
		        pickSoftLight(x + 1, y + 1, z - 1, {0, 0, -1}, {0, 1, 0}),
		        pickSoftLight(x + 1, y + 1, z, {0, 0, -1}, {0, 1, 0})
	        };
	        face(glm::vec3(x + size.x, y, z), size.z, size.y, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), texfaces[1], lights,
	             do_tint(0.8f), rot == 3);
        }
    }

    void BlocksRenderer::blockXSprite(int x, int y, int z, const glm::vec3& size, const UVRegion& face1,
	    const UVRegion& face2, float spread) {
    	glm::vec4 lights[]{
    		pickSoftLight(x, y + 1, z, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight(x + 1, y + 1, z, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight(x + 1, y + 1, z, {1, 0, 0}, {0, 1, 0}),
			pickSoftLight(x, y + 1, z, {1, 0, 0}, {0, 1, 0}) };

    	int rand = ((x * z + y) ^ (z * y - x)) * (z + y);

    	float xs = ((float)(char)rand / 512) * spread;
    	float zs = ((float)(char)(rand >> 8) / 512) * spread;

    	const float w = size.x / 1.41f;
    	face(glm::vec3(x + xs + (1.0 - w) * 0.5f, y,
			z + zs - 1 + (1.0 - w) * 0.5f), w, size.y,
			glm::vec3(1.0f, 0, 1.0f), glm::vec3(0, 1, 0), face1, lights, do_tint(0.8f));
    	face(glm::vec3(x + xs - (1.0 - w) * 0.5f + 1, y,
			z + zs - (1.0 - w) * 0.5f), w, size.y,
			glm::vec3(-1.0f, 0, -1.0f), glm::vec3(0, 1, 0), face1, lights, do_tint(0.8f));

    	face(glm::vec3(x + xs + (1.0 - w) * 0.5f, y,
			z + zs - (1.0 - w) * 0.5f), w, size.y,
			glm::vec3(1.0f, 0, -1.0f), glm::vec3(0, 1, 0), face2, lights, do_tint(0.8f));
    	face(glm::vec3(x + xs - (1.0 - w) * 0.5f + 1, y,
			z + zs + (1.0 - w) * 0.5f - 1), w, size.y,
			glm::vec3(-1.0f, 0, 1.0f), glm::vec3(0, 1, 0), face2, lights, do_tint(0.8f));
    }

    bool BlocksRenderer::isOpenForLight(int x, int y, int z) const {
    	blockid_t id = m_voxelsBuffer->pickBlockId(m_chunk->x * CHUNK_W + x, y, m_chunk->z * CHUNK_D + z);
    	if (id == BLOCK_VOID)
    		return false;
    	const Block& block = *m_blockDefsCache[id];
    	if (block.lightPassing) {
    		return true;
    	}
    	return !id;
    }

    bool BlocksRenderer::isOpen(int x, int y, int z, ubyte group) const {
    	blockid_t id = m_voxelsBuffer->pickBlockId(m_chunk->x * CHUNK_W + x, y, m_chunk->z * CHUNK_D + z);
    	if (id == BLOCK_VOID)
    		return false;
    	const Block& block = *m_blockDefsCache[id];
    	if (block.drawGroup != group && block.lightPassing) {
    		return true;
    	}
    	return !id;
    }

    glm::vec4 BlocksRenderer::pickLight(int x, int y, int z) const {
    	if (isOpenForLight(x, y, z)) {
    		light_t light = m_voxelsBuffer->pickLight(m_chunk->x * CHUNK_W + x, y, m_chunk->z * CHUNK_D + z);
    		return glm::vec4(Lightmap::extract(light, 0) / 15.0f,
				Lightmap::extract(light, 1) / 15.0f,
				Lightmap::extract(light, 2) / 15.0f,
				Lightmap::extract(light, 3) / 15.0f);
    	}

	    return glm::vec4(0.0f);
    }

    glm::vec4 BlocksRenderer::pickSoftLight(int x, int y, int z, const glm::ivec3& right, const glm::ivec3& up) const {
    	return (pickLight(x - right.x - up.x, y - right.y - up.y, z - right.z - up.z) +
			pickLight(x - up.x, y - up.y, z - up.z) +
			pickLight(x, y, z) +
			pickLight(x - right.x, y - right.y, z - right.z)) * 0.25f;
    }

    void BlocksRenderer::render(const voxel* voxels, int atlas_size) {
    	int begin = m_chunk->bottom * (CHUNK_W * CHUNK_D);
    	int end = m_chunk->top * (CHUNK_W * CHUNK_D);
    	for (ubyte group = 0; group < 8; group++) {
    		for (int i = begin; i < end; i++) {
    			const voxel& vox = voxels[i];
    			blockid_t id = vox.id;
    			const Block& def = *m_blockDefsCache[id];
    			if (!id || def.drawGroup != group)
    				continue;
    			const UVRegion texfaces[6]{ m_cache->getRegion(id, 0), m_cache->getRegion(id, 1),
											m_cache->getRegion(id, 2), m_cache->getRegion(id, 3),
											m_cache->getRegion(id, 4), m_cache->getRegion(id, 5)};
    			int x = i % CHUNK_W;
    			int y = i / (CHUNK_D * CHUNK_W);
    			int z = (i / CHUNK_D) % CHUNK_W;
    			switch (def.model) {
    				case BlockModel::block:
    					if (*((uint32_t*)&def.emission)) {
    						blockCube(x, y, z, glm::vec3(1, 1, 1), texfaces, def.drawGroup);
    					}
    					else {
    						blockCubeShaded(x, y, z, glm::vec3(1, 1, 1), texfaces, &def, vox.states);
    					}
    				break;
    				case BlockModel::xsprite: {
    					blockXSprite(x, y, z, glm::vec3(1, 1, 1), texfaces[FACE_MX], texfaces[FACE_MZ], 1.0f);
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

    BlocksRenderer::BlocksRenderer(size_t capacity,
                                   const Content* content,
                                   const ContentGfxCache* cache,
                                   const EngineSettings& settings)
        : m_content(content),
          m_capacity(capacity),
          m_cache(cache),
          m_settings(settings) {
        m_vertexBuffer = new VertexMain[capacity];
        m_indexBuffer = new int[capacity];
        m_voxelsBuffer = new VoxelsVolume(CHUNK_W + 2, CHUNK_H, CHUNK_D + 2);
        m_blockDefsCache = content->indices->getBlockDefs();
    }

    BlocksRenderer::~BlocksRenderer() {
        delete m_voxelsBuffer;
        delete[] m_vertexBuffer;
        delete[] m_indexBuffer;
    }

    Mesh<VertexMain>* BlocksRenderer::render(const Chunk* chunk, int atlas_size, const ChunksStorage* chunks) {
    	m_chunk = chunk;
    	m_voxelsBuffer->setPosition(chunk->x * CHUNK_W - 1, 0, chunk->z * CHUNK_D - 1);
    	chunks->getVoxels(m_voxelsBuffer, m_settings.graphics.backlight);
    	overflow = false;
    	m_vertexOffset = 0;
    	m_indexOffset = m_indexSize = 0;
    	const voxel* voxels = chunk->voxels;
    	render(voxels, atlas_size);

    	Mesh<VertexMain>* mesh = new Mesh(m_vertexBuffer, m_vertexOffset, m_indexBuffer, m_indexSize);
    	return mesh;
    }
} // vulkan