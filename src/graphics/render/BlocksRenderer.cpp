#include "BlocksRenderer.hpp"

#include <graphics/core/Mesh.hpp>
#include <maths/UVRegion.hpp>
#include <constants.hpp>
#include <content/Content.hpp>
#include <voxels/Block.hpp>
#include <voxels/Chunk.hpp>
#include <voxels/VoxelsVolume.hpp>
#include <voxels/ChunksStorage.hpp>
#include <lighting/Lightmap.hpp>
#include <frontend/ContentGfxCache.hpp>
#include <settings.hpp>

#include <glm/glm.hpp>

using glm::ivec3;
using glm::vec3;
using glm::vec4;

const uint BlocksRenderer::VERTEX_SIZE = 6;
const vec3 BlocksRenderer::SUN_VECTOR (0.411934f, 0.863868f, -0.279161f);

BlocksRenderer::BlocksRenderer(
    size_t capacity,
    const Content* content,
    const ContentGfxCache* cache,
    const EngineSettings* settings
) : content(content),
    vertexBuffer(std::make_unique<float[]>(capacity)),
    indexBuffer(std::make_unique<int[]>(capacity)),
    vertexOffset(0),
    indexOffset(0),
    indexSize(0),
    capacity(capacity),
    cache(cache),
    settings(settings) 
{
    voxelsBuffer = std::make_unique<VoxelsVolume>(
        CHUNK_W + voxelBufferPadding*2, 
        CHUNK_H, 
        CHUNK_D + voxelBufferPadding*2);
    blockDefsCache = content->getIndices()->blocks.getDefs();
}

BlocksRenderer::~BlocksRenderer() {
}

/* Basic vertex add method */
void BlocksRenderer::vertex(const vec3& coord, float u, float v, const vec4& light) {
    vertexBuffer[vertexOffset++] = coord.x;
    vertexBuffer[vertexOffset++] = coord.y;
    vertexBuffer[vertexOffset++] = coord.z;

    vertexBuffer[vertexOffset++] = u;
    vertexBuffer[vertexOffset++] = v;

    union {
        float floating;
        uint32_t integer;
    } compressed;

    compressed.integer = (static_cast<uint32_t>(light.r * 255) & 0xff) << 24;
    compressed.integer |= (static_cast<uint32_t>(light.g * 255) & 0xff) << 16;
    compressed.integer |= (static_cast<uint32_t>(light.b * 255) & 0xff) << 8;
    compressed.integer |= (static_cast<uint32_t>(light.a * 255) & 0xff);

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

/// @brief Add face with precalculated lights
void BlocksRenderer::face(
    const vec3& coord, 
    float w, float h, float d,
    const vec3& axisX,
    const vec3& axisY,
    const vec3& axisZ,
    const UVRegion& region,
    const vec4(&lights)[4],
    const vec4& tint
) {
    if (vertexOffset + BlocksRenderer::VERTEX_SIZE * 4 > capacity) {
        overflow = true;
        return;
    }
    vec3 X = axisX * w;
    vec3 Y = axisY * h;
    vec3 Z = axisZ * d;
    float s = 0.5f;
    vertex(coord + (-X - Y + Z) * s, region.u1, region.v1, lights[0] * tint);
    vertex(coord + ( X - Y + Z) * s, region.u2, region.v1, lights[1] * tint);
    vertex(coord + ( X + Y + Z) * s, region.u2, region.v2, lights[2] * tint);
    vertex(coord + (-X + Y + Z) * s, region.u1, region.v2, lights[3] * tint);
    index(0, 1, 3, 1, 2, 3);
}

void BlocksRenderer::vertexAO(
    const vec3& coord, 
    float u, float v,
    const vec4& tint,
    const vec3& axisX,
    const vec3& axisY,
    const vec3& axisZ
) {
    vec3 pos = coord+axisZ*0.5f+(axisX+axisY)*0.5f;
    vec4 light = pickSoftLight(ivec3(round(pos.x), round(pos.y), round(pos.z)), axisX, axisY);
    vertex(coord, u, v, light * tint);
}

void BlocksRenderer::faceAO(
    const vec3& coord,
    const vec3& X,
    const vec3& Y,
    const vec3& Z,
    const UVRegion& region,
    bool lights
) {
    if (vertexOffset + BlocksRenderer::VERTEX_SIZE * 4 > capacity) {
        overflow = true;
        return;
    }

    float s = 0.5f;
    if (lights) {
        float d = glm::dot(glm::normalize(Z), SUN_VECTOR);
        d = 0.8f + d * 0.2f;

        vec3 axisX = glm::normalize(X);
        vec3 axisY = glm::normalize(Y);
        vec3 axisZ = glm::normalize(Z);

        vec4 tint(d);
        vertexAO(coord + (-X - Y + Z) * s, region.u1, region.v1, tint, axisX, axisY, axisZ);
        vertexAO(coord + ( X - Y + Z) * s, region.u2, region.v1, tint, axisX, axisY, axisZ);
        vertexAO(coord + ( X + Y + Z) * s, region.u2, region.v2, tint, axisX, axisY, axisZ);
        vertexAO(coord + (-X + Y + Z) * s, region.u1, region.v2, tint, axisX, axisY, axisZ);
    } else {
        glm::vec4 tint(1.0f);
        vertex(coord + (-X - Y + Z) * s, region.u1, region.v1, tint);
        vertex(coord + ( X - Y + Z) * s, region.u2, region.v1, tint);
        vertex(coord + ( X + Y + Z) * s, region.u2, region.v2, tint);
        vertex(coord + (-X + Y + Z) * s, region.u1, region.v2, tint);
    }
    index(0, 1, 2, 0, 2, 3);
}

void BlocksRenderer::face(
    const vec3& coord,
    const vec3& X,
    const vec3& Y,
    const vec3& Z,
    const UVRegion& region,
    vec4 tint,
    bool lights
) {
    if (vertexOffset + BlocksRenderer::VERTEX_SIZE * 4 > capacity) {
        overflow = true;
        return;
    }

    float s = 0.5f;
    if (lights) {
        float d = glm::dot(glm::normalize(Z), SUN_VECTOR);
        d = 0.8f + d * 0.2f;
        tint *= d;
    }
    vertex(coord + (-X - Y + Z) * s, region.u1, region.v1, tint);
    vertex(coord + ( X - Y + Z) * s, region.u2, region.v1, tint);
    vertex(coord + ( X + Y + Z) * s, region.u2, region.v2, tint);
    vertex(coord + (-X + Y + Z) * s, region.u1, region.v2, tint);
    index(0, 1, 2, 0, 2, 3);
}

void BlocksRenderer::tetragonicFace(
    const vec3& coord, 
    const vec3& p1, const vec3& p2, const vec3& p3, const vec3& p4,
    const vec3& X, const vec3& Y, const vec3& Z,
    const UVRegion& texreg,
    bool lights
) {    
    const vec3 fp1 = (p1.x - 0.5f) * X + (p1.y - 0.5f) * Y + (p1.z - 0.5f) * Z;
    const vec3 fp2 = (p2.x - 0.5f) * X + (p2.y - 0.5f) * Y + (p2.z - 0.5f) * Z;
    const vec3 fp3 = (p3.x - 0.5f) * X + (p3.y - 0.5f) * Y + (p3.z - 0.5f) * Z;
    const vec3 fp4 = (p4.x - 0.5f) * X + (p4.y - 0.5f) * Y + (p4.z - 0.5f) * Z;

    vec4 tint(1.0f);
    if (lights) {
        vec3 dir = glm::cross(fp2 - fp1, fp3 - fp1);
        vec3 normal = glm::normalize(dir);

        float d = glm::dot(normal, SUN_VECTOR);
        d = 0.8f + d * 0.2f;
        tint *= d;
        tint *= pickLight(coord);
        // debug normal
        // tint.x = normal.x * 0.5f + 0.5f;
        // tint.y = normal.y * 0.5f + 0.5f;
        // tint.z = normal.z * 0.5f + 0.5f;
    }
    vertex(coord + fp1, texreg.u1, texreg.v1, tint);
    vertex(coord + fp2, texreg.u2, texreg.v1, tint);
    vertex(coord + fp3, texreg.u2, texreg.v2, tint);
    vertex(coord + fp4, texreg.u1, texreg.v2, tint);
    index(0, 1, 3, 1, 2, 3);
}

void BlocksRenderer::blockXSprite(
    int x, int y, int z, 
    const vec3& size, 
    const UVRegion& texface1, 
    const UVRegion& texface2, 
    float spread
) {
    vec4 lights[] {
        pickSoftLight({x, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
        pickSoftLight({x + 1, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
        pickSoftLight({x + 1, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
        pickSoftLight({x, y + 1, z}, {1, 0, 0}, {0, 1, 0})
    };
    int rand = ((x * z + y) ^ (z * y - x)) * (z + y);

    float xs = ((float)(char)rand / 512) * spread;
    float zs = ((float)(char)(rand >> 8) / 512) * spread;

    const float w = size.x / 1.41f;
    const float tint = 0.8f;

    face(vec3(x + xs, y, z + zs), 
        w, size.y, 0, vec3(1, 0, 1), vec3(0, 1, 0), vec3(),
        texface1, lights, vec4(tint));
    face(vec3(x + xs, y, z + zs), 
        w, size.y, 0, vec3(-1, 0, -1), vec3(0, 1, 0), vec3(), 
        texface1, lights, vec4(tint));

    face(vec3(x + xs, y, z + zs), 
        w, size.y, 0, vec3(1, 0, -1), vec3(0, 1, 0), vec3(), 
        texface2, lights, vec4(tint));
    face(vec3(x + xs, y, z + zs), 
        w, size.y, 0, vec3(-1, 0, 1), vec3(0, 1, 0), vec3(), 
        texface2, lights, vec4(tint));
}

// HINT: texture faces order: {east, west, bottom, top, south, north}

/// @brief AABB blocks render method
void BlocksRenderer::blockAABB(
    const ivec3& icoord,
    const UVRegion(&texfaces)[6], 
    const Block* block, 
    ubyte rotation,
    bool lights,
    bool ao
) {
    if (block->hitboxes.empty()) {
        return;
    }
    AABB hitbox = block->hitboxes[0];
    for (const auto& box : block->hitboxes) {
        hitbox.a = glm::min(hitbox.a, box.a);
        hitbox.b = glm::max(hitbox.b, box.b);
    }
    vec3 size = hitbox.size();
    vec3 X(1, 0, 0);
    vec3 Y(0, 1, 0);
    vec3 Z(0, 0, 1);
    vec3 coord(icoord);
    if (block->rotatable) {
        auto& rotations = block->rotations;
        auto& orient = rotations.variants[rotation];
        X = orient.axisX;
        Y = orient.axisY;
        Z = orient.axisZ;
        orient.transform(hitbox);
    }
    coord = vec3(icoord) - vec3(0.5f) + hitbox.center();
    
    if (ao) {
        faceAO(coord,  X*size.x,  Y*size.y,  Z*size.z, texfaces[5], lights); // north
        faceAO(coord, -X*size.x,  Y*size.y, -Z*size.z, texfaces[4], lights); // south

        faceAO(coord,  X*size.x, -Z*size.z,  Y*size.y, texfaces[3], lights); // top
        faceAO(coord, -X*size.x, -Z*size.z, -Y*size.y, texfaces[2], lights); // bottom

        faceAO(coord, -Z*size.z,  Y*size.y,  X*size.x, texfaces[1], lights); // west
        faceAO(coord,  Z*size.z,  Y*size.y, -X*size.x, texfaces[0], lights); // east
    } else {
        auto tint = pickLight(icoord);
        face(coord,  X*size.x,  Y*size.y,  Z*size.z, texfaces[5], tint, lights); // north
        face(coord, -X*size.x,  Y*size.y, -Z*size.z, texfaces[4], tint, lights); // south

        face(coord,  X*size.x, -Z*size.z,  Y*size.y, texfaces[3], tint, lights); // top
        face(coord, -X*size.x, -Z*size.z, -Y*size.y, texfaces[2], tint, lights); // bottom

        face(coord, -Z*size.z,  Y*size.y,  X*size.x, texfaces[1], tint, lights); // west
        face(coord,  Z*size.z,  Y*size.y, -X*size.x, texfaces[0], tint, lights); // east
    }
}

void BlocksRenderer::blockCustomModel(
    const ivec3& icoord, const Block* block, ubyte rotation, bool lights, bool ao
) {
    vec3 X(1, 0, 0);
    vec3 Y(0, 1, 0);
    vec3 Z(0, 0, 1);
    CoordSystem orient(X,Y,Z);
    vec3 coord(icoord);
    if (block->rotatable) {
        auto& rotations = block->rotations;
        orient = rotations.variants[rotation];
        X = orient.axisX;
        Y = orient.axisY;
        Z = orient.axisZ;
    }

    for (size_t i = 0; i < block->modelBoxes.size(); i++) {
        AABB box = block->modelBoxes[i];
        vec3 size = box.size();
        if (block->rotatable) {
            orient.transform(box);
        }
        vec3 center_coord = coord - vec3(0.5f) + box.center();
        faceAO(center_coord, X * size.x, Y * size.y, Z * size.z, block->modelUVs[i * 6 + 5], lights); // north
        faceAO(center_coord, -X * size.x, Y * size.y, -Z * size.z, block->modelUVs[i * 6 + 4], lights); // south
        faceAO(center_coord, X * size.x, -Z * size.z, Y * size.y, block->modelUVs[i * 6 + 3], lights); // top
        faceAO(center_coord, -X * size.x, -Z * size.z, -Y * size.y, block->modelUVs[i * 6 + 2], lights); // bottom
        faceAO(center_coord, -Z * size.z, Y * size.y, X * size.x, block->modelUVs[i * 6 + 1], lights); // west
        faceAO(center_coord, Z * size.z, Y * size.y, -X * size.x, block->modelUVs[i * 6 + 0], lights); // east
    }
    
    for (size_t i = 0; i < block->modelExtraPoints.size()/4; i++) {
        tetragonicFace(coord,
            block->modelExtraPoints[i * 4 + 0],
            block->modelExtraPoints[i * 4 + 1],
            block->modelExtraPoints[i * 4 + 2],
            block->modelExtraPoints[i * 4 + 3],
            X, Y, Z,
            block->modelUVs[block->modelBoxes.size()*6 + i], lights);
    }
}

/* Fastest solid shaded blocks render method */
void BlocksRenderer::blockCube(
    int x, int y, int z, 
    const UVRegion(&texfaces)[6], 
    const Block* block, 
    blockstate states,
    bool lights,
    bool ao
) {
    ubyte group = block->drawGroup;

    vec3 X(1, 0, 0);
    vec3 Y(0, 1, 0);
    vec3 Z(0, 0, 1);
    vec3 coord(x, y, z);
    if (block->rotatable) {
        auto& rotations = block->rotations;
        auto& orient = rotations.variants[states.rotation];
        X = orient.axisX;
        Y = orient.axisY;
        Z = orient.axisZ;
    }
    
    if (ao) {
        if (isOpen(x+Z.x, y+Z.y, z+Z.z, group)) {
            faceAO(coord, X, Y, Z, texfaces[5], lights);
        }
        if (isOpen(x-Z.x, y-Z.y, z-Z.z, group)) {
            faceAO(coord, -X, Y, -Z, texfaces[4], lights);
        }
        if (isOpen(x+Y.x, y+Y.y, z+Y.z, group)) {
            faceAO(coord, X, -Z, Y, texfaces[3], lights);
        }
        if (isOpen(x-Y.x, y-Y.y, z-Y.z, group)) {
            faceAO(coord, X, Z, -Y, texfaces[2], lights);
        }
        if (isOpen(x+X.x, y+X.y, z+X.z, group)) {
            faceAO(coord, -Z, Y, X, texfaces[1], lights);
        }
        if (isOpen(x-X.x, y-X.y, z-X.z, group)) {
            faceAO(coord, Z, Y, -X, texfaces[0], lights);
        }
    } else {
        if (isOpen(x+Z.x, y+Z.y, z+Z.z, group)) {
            face(coord, X, Y, Z, texfaces[5], pickLight({x, y, z+1}), lights);
        }
        if (isOpen(x-Z.x, y-Z.y, z-Z.z, group)) {
            face(coord, -X, Y, -Z, texfaces[4], pickLight({x, y, z-1}), lights);
        }
        if (isOpen(x+Y.x, y+Y.y, z+Y.z, group)) {
            face(coord, X, -Z, Y, texfaces[3], pickLight({x, y+1, z}), lights);
        }
        if (isOpen(x-Y.x, y-Y.y, z-Y.z, group)) {
            face(coord, X, Z, -Y, texfaces[2], pickLight({x, y-1, z}), lights);
        }
        if (isOpen(x+X.x, y+X.y, z+X.z, group)) {
            face(coord, -Z, Y, X, texfaces[1], pickLight({x+1, y, z}), lights);
        }
        if (isOpen(x-X.x, y-X.y, z-X.z, group)) {
            face(coord, Z, Y, -X, texfaces[0], pickLight({x-1, y, z}), lights);
        }
    }
}

// Does block allow to see other blocks sides (is it transparent)
bool BlocksRenderer::isOpen(int x, int y, int z, ubyte group) const {
    blockid_t id = voxelsBuffer->pickBlockId(chunk->x * CHUNK_W + x, 
                                             y, 
                                             chunk->z * CHUNK_D + z);
    if (id == BLOCK_VOID) {
        return false;
    }
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
    if (id == BLOCK_VOID) {
        return false;
    }
    const Block& block = *blockDefsCache[id];
    if (block.lightPassing) {
        return true;
    }
    return !id;
}

vec4 BlocksRenderer::pickLight(int x, int y, int z) const {
    if (isOpenForLight(x, y, z)) {
        light_t light = voxelsBuffer->pickLight(chunk->x * CHUNK_W + x, y, 
                                                chunk->z * CHUNK_D + z);
        return vec4(Lightmap::extract(light, 0) / 15.0f,
                    Lightmap::extract(light, 1) / 15.0f,
                    Lightmap::extract(light, 2) / 15.0f,
                    Lightmap::extract(light, 3) / 15.0f);
    } else {
        return vec4(0.0f);
    }
}

vec4 BlocksRenderer::pickLight(const ivec3& coord) const {
    return pickLight(coord.x, coord.y, coord.z);
}

vec4 BlocksRenderer::pickSoftLight(const ivec3& coord, 
                                   const ivec3& right, 
                                   const ivec3& up) const {
    return (pickLight(coord) +
            pickLight(coord - right) +
            pickLight(coord - right - up) +
            pickLight(coord - up)) * 0.25f;
}

vec4 BlocksRenderer::pickSoftLight(float x, float y, float z, 
                                   const ivec3& right, 
                                   const ivec3& up) const {
    return pickSoftLight({
        static_cast<int>(round(x)), 
        static_cast<int>(round(y)), 
        static_cast<int>(round(z))},
        right, up);
}

void BlocksRenderer::render(const voxel* voxels) {
    int begin = chunk->bottom * (CHUNK_W * CHUNK_D);
    int end = chunk->top * (CHUNK_W * CHUNK_D);
    for (const auto drawGroup : *content->drawGroups) {
        for (int i = begin; i < end; i++) {
            const voxel& vox = voxels[i];
            blockid_t id = vox.id;
            blockstate state = vox.state;
            const Block& def = *blockDefsCache[id];
            if (id == 0 || def.drawGroup != drawGroup || state.segment) {
                continue;
            }
            const UVRegion texfaces[6] {
                cache->getRegion(id, 0), 
                cache->getRegion(id, 1),
                cache->getRegion(id, 2), 
                cache->getRegion(id, 3),
                cache->getRegion(id, 4), 
                cache->getRegion(id, 5)
            };
            int x = i % CHUNK_W;
            int y = i / (CHUNK_D * CHUNK_W);
            int z = (i / CHUNK_D) % CHUNK_W;
            switch (def.model) {
                case BlockModel::block:
                    blockCube(x, y, z, texfaces, &def, vox.state, !def.shadeless,
                              def.ambientOcclusion);
                    break;
                case BlockModel::xsprite: {
                    blockXSprite(x, y, z, vec3(1.0f), 
                                texfaces[FACE_MX], texfaces[FACE_MZ], 1.0f);
                    break;
                }
                case BlockModel::aabb: {
                    blockAABB(ivec3(x,y,z), texfaces, &def, vox.state.rotation, 
                              !def.shadeless, def.ambientOcclusion);
                    break;
                }
                case BlockModel::custom: {
                    blockCustomModel(ivec3(x, y, z), &def, vox.state.rotation, 
                                     !def.shadeless, def.ambientOcclusion);
                    break;
                }
                default:
                    break;
            }
            if (overflow) {
                return;
            }
        }
    }
}

void BlocksRenderer::build(const Chunk* chunk, const ChunksStorage* chunks) {
    this->chunk = chunk;
    voxelsBuffer->setPosition(
        chunk->x * CHUNK_W - voxelBufferPadding, 0,
        chunk->z * CHUNK_D - voxelBufferPadding);
    chunks->getVoxels(voxelsBuffer.get(), settings->graphics.backlight.get());
    overflow = false;
    vertexOffset = 0;
    indexOffset = indexSize = 0;
    const voxel* voxels = chunk->voxels;
    render(voxels);
}

std::shared_ptr<Mesh> BlocksRenderer::createMesh() {
    const vattr attrs[]{ {3}, {2}, {1}, {0} };
    size_t vcount = vertexOffset / BlocksRenderer::VERTEX_SIZE;
    return std::make_shared<Mesh>(
        vertexBuffer.get(), vcount, indexBuffer.get(), indexSize, attrs
    );
}

std::shared_ptr<Mesh> BlocksRenderer::render(const Chunk* chunk, const ChunksStorage* chunks) {
    build(chunk, chunks);
    return createMesh();
}

VoxelsVolume* BlocksRenderer::getVoxelsBuffer() const {
    return voxelsBuffer.get();
}
