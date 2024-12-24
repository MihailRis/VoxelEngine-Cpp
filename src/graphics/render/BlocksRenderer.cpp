#include "BlocksRenderer.hpp"

#include "graphics/core/Mesh.hpp"
#include "graphics/commons/Model.hpp"
#include "maths/UVRegion.hpp"
#include "constants.hpp"
#include "content/Content.hpp"
#include "voxels/Chunks.hpp"
#include "lighting/Lightmap.hpp"
#include "frontend/ContentGfxCache.hpp"

const glm::vec3 BlocksRenderer::SUN_VECTOR (0.411934f, 0.863868f, -0.279161f);

BlocksRenderer::BlocksRenderer(
    size_t capacity,
    const Content& content,
    const ContentGfxCache& cache,
    const EngineSettings& settings
) : content(content),
    vertexBuffer(std::make_unique<float[]>(capacity * CHUNK_VERTEX_SIZE)),
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
    blockDefsCache = content.getIndices()->blocks.getDefs();
}

BlocksRenderer::~BlocksRenderer() {
}

/// Basic vertex add method
void BlocksRenderer::vertex(
    const glm::vec3& coord, float u, float v, const glm::vec4& light
) {
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
    const glm::vec3& coord, 
    float w, float h, float d,
    const glm::vec3& axisX,
    const glm::vec3& axisY,
    const glm::vec3& axisZ,
    const UVRegion& region,
    const glm::vec4(&lights)[4],
    const glm::vec4& tint
) {
    if (vertexOffset + CHUNK_VERTEX_SIZE * 4 > capacity) {
        overflow = true;
        return;
    }
    auto X = axisX * w;
    auto Y = axisY * h;
    auto Z = axisZ * d;
    float s = 0.5f;
    vertex(coord + (-X - Y + Z) * s, region.u1, region.v1, lights[0] * tint);
    vertex(coord + ( X - Y + Z) * s, region.u2, region.v1, lights[1] * tint);
    vertex(coord + ( X + Y + Z) * s, region.u2, region.v2, lights[2] * tint);
    vertex(coord + (-X + Y + Z) * s, region.u1, region.v2, lights[3] * tint);
    index(0, 1, 3, 1, 2, 3);
}

void BlocksRenderer::vertexAO(
    const glm::vec3& coord, 
    float u, float v,
    const glm::vec4& tint,
    const glm::vec3& axisX,
    const glm::vec3& axisY,
    const glm::vec3& axisZ
) {
    auto pos = coord+axisZ*0.5f+(axisX+axisY)*0.5f;
    auto light = pickSoftLight(
        glm::ivec3(std::round(pos.x), std::round(pos.y), std::round(pos.z)),
        axisX,
        axisY
    );
    vertex(coord, u, v, light * tint);
}

void BlocksRenderer::faceAO(
    const glm::vec3& coord,
    const glm::vec3& X,
    const glm::vec3& Y,
    const glm::vec3& Z,
    const UVRegion& region,
    bool lights
) {
    if (vertexOffset + CHUNK_VERTEX_SIZE * 4 > capacity) {
        overflow = true;
        return;
    }

    float s = 0.5f;
    if (lights) {
        float d = glm::dot(glm::normalize(Z), SUN_VECTOR);
        d = 0.8f + d * 0.2f;

        auto axisX = glm::normalize(X);
        auto axisY = glm::normalize(Y);
        auto axisZ = glm::normalize(Z);

        glm::vec4 tint(d);
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
    const glm::vec3& coord,
    const glm::vec3& X,
    const glm::vec3& Y,
    const glm::vec3& Z,
    const UVRegion& region,
    glm::vec4 tint,
    bool lights
) {
    if (vertexOffset + CHUNK_VERTEX_SIZE * 4 > capacity) {
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

void BlocksRenderer::blockXSprite(
    int x, int y, int z, 
    const glm::vec3& size, 
    const UVRegion& texface1, 
    const UVRegion& texface2, 
    float spread
) {
    glm::vec4 lights[] {
        pickSoftLight({x, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
        pickSoftLight({x + 1, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
        pickSoftLight({x + 1, y + 1, z}, {1, 0, 0}, {0, 1, 0}),
        pickSoftLight({x, y + 1, z}, {1, 0, 0}, {0, 1, 0})
    };
    randomizer.setSeed((x * 52321) ^ (z * 389) ^ y);
    short rand = randomizer.rand32();

    float xs = ((float)(char)rand / 512) * spread;
    float zs = ((float)(char)(rand >> 8) / 512) * spread;

    const float w = size.x / 1.41f;
    const glm::vec4 tint (0.8f);

    face({x + xs, y, z + zs}, w, size.y, 0, {1, 0, 1}, {0, 1, 0}, glm::vec3(),
        texface1, lights, tint);
    face({x + xs, y, z + zs}, w, size.y, 0, {-1, 0, -1}, {0, 1, 0}, glm::vec3(), 
        texface1, lights, tint);

    face({x + xs, y, z + zs}, w, size.y, 0, {1, 0, -1}, {0, 1, 0}, glm::vec3(), 
        texface2, lights, tint);
    face({x + xs, y, z + zs}, w, size.y, 0, {-1, 0, 1}, {0, 1, 0}, glm::vec3(), 
        texface2, lights, tint);
}

// HINT: texture faces order: {east, west, bottom, top, south, north}

/// @brief AABB blocks render method
void BlocksRenderer::blockAABB(
    const glm::ivec3& icoord,
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
    auto size = hitbox.size();
    glm::vec3 X(1, 0, 0);
    glm::vec3 Y(0, 1, 0);
    glm::vec3 Z(0, 0, 1);
    glm::vec3 coord(icoord);
    if (block->rotatable) {
        auto& rotations = block->rotations;
        auto& orient = rotations.variants[rotation];
        X = orient.axisX;
        Y = orient.axisY;
        Z = orient.axisZ;
        orient.transform(hitbox);
    }
    coord -= glm::vec3(0.5f) - hitbox.center();
    
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
    const glm::ivec3& icoord, const Block* block, ubyte rotation, bool lights, bool ao
) {
    glm::vec3 X(1, 0, 0);
    glm::vec3 Y(0, 1, 0);
    glm::vec3 Z(0, 0, 1);
    CoordSystem orient(X,Y,Z);
    glm::vec3 coord(icoord);
    if (block->rotatable) {
        auto& rotations = block->rotations;
        orient = rotations.variants[rotation];
        X = orient.axisX;
        Y = orient.axisY;
        Z = orient.axisZ;
    }

    const auto& model = cache.getModel(block->rt.id);
    for (const auto& mesh : model.meshes) {
        if (vertexOffset + CHUNK_VERTEX_SIZE * mesh.vertices.size() > capacity) {
            overflow = true;
            return;
        }
        for (int triangle = 0; triangle < mesh.vertices.size() / 3; triangle++) {
            auto r = mesh.vertices[triangle * 3 + (triangle % 2) * 2].coord -
                     mesh.vertices[triangle * 3 + 1].coord;
            r = glm::normalize(r);

            for (int i = 0; i < 3; i++) {
                const auto& vertex = mesh.vertices[triangle * 3 + i];
                auto n = vertex.normal.x * X + vertex.normal.y * Y +
                         vertex.normal.z * Z;
                float d = glm::dot(n, SUN_VECTOR);
                d = 0.8f + d * 0.2f;
                const auto& vcoord = vertex.coord - 0.5f;
                vertexAO(
                    coord + vcoord.x * X + vcoord.y * Y + vcoord.z * Z,
                    vertex.uv.x,
                    vertex.uv.y,
                    glm::vec4(d, d, d, d),
                    glm::cross(r, n),
                    r,
                    n
                );
                indexBuffer[indexSize++] = indexOffset++;
            }
        }
    }
}

/* Fastest solid shaded blocks render method */
void BlocksRenderer::blockCube(
    const glm::ivec3& coord, 
    const UVRegion(&texfaces)[6], 
    const Block& block, 
    blockstate states,
    bool lights,
    bool ao
) {
    ubyte group = block.drawGroup;

    glm::ivec3 X(1, 0, 0);
    glm::ivec3 Y(0, 1, 0);
    glm::ivec3 Z(0, 0, 1);

    if (block.rotatable) {
        auto& rotations = block.rotations;
        auto& orient = rotations.variants[states.rotation];
        X = orient.axisX;
        Y = orient.axisY;
        Z = orient.axisZ;
    }
    
    if (ao) {
        if (isOpen(coord + Z, block)) {
            faceAO(coord, X, Y, Z, texfaces[5], lights);
        }
        if (isOpen(coord - Z, block)) {
            faceAO(coord, -X, Y, -Z, texfaces[4], lights);
        }
        if (isOpen(coord + Y, block)) {
            faceAO(coord, X, -Z, Y, texfaces[3], lights);
        }
        if (isOpen(coord - Y, block)) {
            faceAO(coord, X, Z, -Y, texfaces[2], lights);
        }
        if (isOpen(coord + X, block)) {
            faceAO(coord, -Z, Y, X, texfaces[1], lights);
        }
        if (isOpen(coord - X, block)) {
            faceAO(coord, Z, Y, -X, texfaces[0], lights);
        }
    } else {
        if (isOpen(coord + Z, block)) {
            face(coord, X, Y, Z, texfaces[5], pickLight(coord + Z), lights);
        }
        if (isOpen(coord - Z, block)) {
            face(coord, -X, Y, -Z, texfaces[4], pickLight(coord - Z), lights);
        }
        if (isOpen(coord + Y, block)) {
            face(coord, X, -Z, Y, texfaces[3], pickLight(coord + Y), lights);
        }
        if (isOpen(coord - Y, block)) {
            face(coord, X, Z, -Y, texfaces[2], pickLight(coord - Y), lights);
        }
        if (isOpen(coord + X, block)) {
            face(coord, -Z, Y, X, texfaces[1], pickLight(coord + X), lights);
        }
        if (isOpen(coord - X, block)) {
            face(coord, Z, Y, -X, texfaces[0], pickLight(coord - X), lights);
        }
    }
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

glm::vec4 BlocksRenderer::pickLight(int x, int y, int z) const {
    if (isOpenForLight(x, y, z)) {
        light_t light = voxelsBuffer->pickLight(chunk->x * CHUNK_W + x, y, 
                                                chunk->z * CHUNK_D + z);
        return glm::vec4(Lightmap::extract(light, 0),
                         Lightmap::extract(light, 1),
                         Lightmap::extract(light, 2),
                         Lightmap::extract(light, 3)) / 15.0f;
    } else {
        return glm::vec4(0.0f);
    }
}

glm::vec4 BlocksRenderer::pickLight(const glm::ivec3& coord) const {
    return pickLight(coord.x, coord.y, coord.z);
}

glm::vec4 BlocksRenderer::pickSoftLight(
    const glm::ivec3& coord, const glm::ivec3& right, const glm::ivec3& up
) const {
    return (pickLight(coord) +
            pickLight(coord - right) +
            pickLight(coord - right - up) +
            pickLight(coord - up)) * 0.25f;
}

glm::vec4 BlocksRenderer::pickSoftLight(
    float x, float y, float z, const glm::ivec3& right, const glm::ivec3& up
) const {
    return pickSoftLight({
        static_cast<int>(std::round(x)), 
        static_cast<int>(std::round(y)), 
        static_cast<int>(std::round(z))},
        right, up);
}

void BlocksRenderer::render(
    const voxel* voxels, int beginEnds[256][2]
) {
    for (const auto drawGroup : *content.drawGroups) {
        int begin = beginEnds[drawGroup][0];
        if (begin == 0) {
            continue;
        }
        int end = beginEnds[drawGroup][1];
        for (int i = begin-1; i <= end; i++) {
            const voxel& vox = voxels[i];
            blockid_t id = vox.id;
            blockstate state = vox.state;
            const auto& def = *blockDefsCache[id];
            if (id == 0 || def.drawGroup != drawGroup || state.segment) {
                continue;
            }
            if (def.translucent) {
                continue;
            }
            const UVRegion texfaces[6] {
                cache.getRegion(id, 0), cache.getRegion(id, 1),
                cache.getRegion(id, 2), cache.getRegion(id, 3),
                cache.getRegion(id, 4), cache.getRegion(id, 5)
            };
            int x = i % CHUNK_W;
            int y = i / (CHUNK_D * CHUNK_W);
            int z = (i / CHUNK_D) % CHUNK_W;
            switch (def.model) {
                case BlockModel::block:
                    blockCube({x, y, z}, texfaces, def, vox.state, !def.shadeless,
                              def.ambientOcclusion);
                    break;
                case BlockModel::xsprite: {
                    blockXSprite(x, y, z, glm::vec3(1.0f), 
                                texfaces[FACE_MX], texfaces[FACE_MZ], 1.0f);
                    break;
                }
                case BlockModel::aabb: {
                    blockAABB({x, y, z}, texfaces, &def, vox.state.rotation, 
                              !def.shadeless, def.ambientOcclusion);
                    break;
                }
                case BlockModel::custom: {
                    blockCustomModel({x, y, z}, &def, vox.state.rotation, 
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

SortingMeshData BlocksRenderer::renderTranslucent(
    const voxel* voxels, int beginEnds[256][2]
) {
    SortingMeshData sortingMesh {{}};

    AABB aabb {};
    bool aabbInit = false;
    size_t totalSize = 0;
    for (const auto drawGroup : *content.drawGroups) {
        int begin = beginEnds[drawGroup][0];
        if (begin == 0) {
            continue;
        }
        int end = beginEnds[drawGroup][1];
        for (int i = begin-1; i <= end; i++) {
            const voxel& vox = voxels[i];
            blockid_t id = vox.id;
            blockstate state = vox.state;
            const auto& def = *blockDefsCache[id];
            if (id == 0 || def.drawGroup != drawGroup || state.segment) {
                continue;
            }
            if (!def.translucent) {
                continue;
            }
            const UVRegion texfaces[6] {
                cache.getRegion(id, 0), cache.getRegion(id, 1),
                cache.getRegion(id, 2), cache.getRegion(id, 3),
                cache.getRegion(id, 4), cache.getRegion(id, 5)
            };
            int x = i % CHUNK_W;
            int y = i / (CHUNK_D * CHUNK_W);
            int z = (i / CHUNK_D) % CHUNK_W;
            switch (def.model) {
                case BlockModel::block:
                    blockCube({x, y, z}, texfaces, def, vox.state, !def.shadeless,
                              def.ambientOcclusion);
                    break;
                case BlockModel::xsprite: {
                    blockXSprite(x, y, z, glm::vec3(1.0f), 
                                texfaces[FACE_MX], texfaces[FACE_MZ], 1.0f);
                    break;
                }
                case BlockModel::aabb: {
                    blockAABB({x, y, z}, texfaces, &def, vox.state.rotation, 
                              !def.shadeless, def.ambientOcclusion);
                    break;
                }
                case BlockModel::custom: {
                    blockCustomModel({x, y, z}, &def, vox.state.rotation, 
                                     !def.shadeless, def.ambientOcclusion);
                    break;
                }
                default:
                    break;
            }
            if (vertexOffset == 0) {
                continue;
            }
            SortingMeshEntry entry {
                glm::vec3(
                    x + chunk->x * CHUNK_W + 0.5f,
                    y + 0.5f,
                    z + chunk->z * CHUNK_D + 0.5f
                ),
                util::Buffer<float>(indexSize * CHUNK_VERTEX_SIZE)};

            totalSize += entry.vertexData.size();

            for (int j = 0; j < indexSize; j++) {
                std::memcpy(
                    entry.vertexData.data() + j * CHUNK_VERTEX_SIZE,
                    vertexBuffer.get() + indexBuffer[j] * CHUNK_VERTEX_SIZE,
                    sizeof(float) * CHUNK_VERTEX_SIZE
                );
                float& vx = entry.vertexData[j * CHUNK_VERTEX_SIZE + 0];
                float& vy = entry.vertexData[j * CHUNK_VERTEX_SIZE + 1];
                float& vz = entry.vertexData[j * CHUNK_VERTEX_SIZE + 2];

                if (!aabbInit) {
                    aabbInit = true;
                    aabb.a = aabb.b = {vx, vy, vz};
                } else {
                    aabb.addPoint(glm::vec3(vx, vy, vz));
                }
                vx += chunk->x * CHUNK_W + 0.5f;
                vy += 0.5f;
                vz += chunk->z * CHUNK_D + 0.5f;
            }
            sortingMesh.entries.push_back(std::move(entry));
            vertexOffset = 0;
            indexOffset = indexSize = 0;
        }
    }

    // additional powerful optimization
    auto size = aabb.size();
    if ((size.y < 0.01f || size.x < 0.01f || size.z < 0.01f) && 
         sortingMesh.entries.size() > 1) {
        SortingMeshEntry newEntry {
            sortingMesh.entries[0].position,
            util::Buffer<float>(totalSize)
        };
        size_t offset = 0;
        for (const auto& entry : sortingMesh.entries) {
            std::memcpy(
                newEntry.vertexData.data() + offset,
                entry.vertexData.data(), entry.vertexData.size() * sizeof(float)
            );
            offset += entry.vertexData.size();
        }
        return SortingMeshData {{std::move(newEntry)}};
    }
    return sortingMesh;
}

void BlocksRenderer::build(const Chunk* chunk, const Chunks* chunks) {
    this->chunk = chunk;
    voxelsBuffer->setPosition(
        chunk->x * CHUNK_W - voxelBufferPadding, 0,
        chunk->z * CHUNK_D - voxelBufferPadding);
    chunks->getVoxels(voxelsBuffer.get(), settings.graphics.backlight.get());

    if (voxelsBuffer->pickBlockId(
        chunk->x * CHUNK_W, 0, chunk->z * CHUNK_D
    ) == BLOCK_VOID) {
        cancelled = true;
        return;
    }
    const voxel* voxels = chunk->voxels;

    int totalBegin = chunk->bottom * (CHUNK_W * CHUNK_D);
    int totalEnd = chunk->top * (CHUNK_W * CHUNK_D);

    int beginEnds[256][2] {};
    for (int i = totalBegin; i < totalEnd; i++) {
        const voxel& vox = voxels[i];
        blockid_t id = vox.id;
        const auto& def = *blockDefsCache[id];
    
        if (beginEnds[def.drawGroup][0] == 0) {
            beginEnds[def.drawGroup][0] = i+1;
        }
        beginEnds[def.drawGroup][1] = i;
    }
    cancelled = false;

    overflow = false;
    vertexOffset = 0;
    indexOffset = indexSize = 0;
    
    sortingMesh = std::move(renderTranslucent(voxels, beginEnds));
    
    overflow = false;
    vertexOffset = 0;
    indexOffset = indexSize = 0;
    
    render(voxels, beginEnds);
}

ChunkMeshData BlocksRenderer::createMesh() {
    return ChunkMeshData {
        MeshData(
            util::Buffer<float>(vertexBuffer.get(), vertexOffset),
            util::Buffer<int>(indexBuffer.get(), indexSize),
            util::Buffer<VertexAttribute>(
                CHUNK_VATTRS, sizeof(CHUNK_VATTRS) / sizeof(VertexAttribute)
            )
        ),
        std::move(sortingMesh)};
}

ChunkMesh BlocksRenderer::render(const Chunk* chunk, const Chunks* chunks) {
    build(chunk, chunks);

    size_t vcount = vertexOffset / CHUNK_VERTEX_SIZE;
    return ChunkMesh{std::make_unique<Mesh>(
        vertexBuffer.get(), vcount, indexBuffer.get(), indexSize, CHUNK_VATTRS
    ), std::move(sortingMesh)};
}

VoxelsVolume* BlocksRenderer::getVoxelsBuffer() const {
    return voxelsBuffer.get();
}
