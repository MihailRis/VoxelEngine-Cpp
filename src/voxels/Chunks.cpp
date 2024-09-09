#include "Chunks.hpp"

#include <limits.h>
#include <math.h>

#include <algorithm>
#include <vector>

#include "coders/byte_utils.hpp"
#include "coders/json.hpp"
#include "content/Content.hpp"
#include "files/WorldFiles.hpp"
#include "graphics/core/Mesh.hpp"
#include "lighting/Lightmap.hpp"
#include "maths/aabb.hpp"
#include "maths/rays.hpp"
#include "maths/voxmaths.hpp"
#include "objects/Entities.hpp"
#include "world/Level.hpp"
#include "world/LevelEvents.hpp"
#include "Block.hpp"
#include "Chunk.hpp"
#include "voxel.hpp"

Chunks::Chunks(
    int32_t w,
    int32_t d,
    int32_t ox,
    int32_t oz,
    WorldFiles* wfile,
    Level* level
)
    : level(level),
      indices(level->content->getIndices()),
      areaMap(w, d),
      worldFiles(wfile) {
    areaMap.setCenter(ox-w/2, oz-d/2);
    areaMap.setOutCallback([this](const auto& chunk) {
        save(chunk.get());
    });
}

voxel* Chunks::get(int32_t x, int32_t y, int32_t z) const {
    if (y < 0 || y >= CHUNK_H) {
        return nullptr;
    }
    int cx = floordiv(x, CHUNK_W);
    int cz = floordiv(z, CHUNK_D);
    auto ptr = areaMap.getIf(cx, cz);
    if (ptr == nullptr) {
        return nullptr;
    }
    Chunk* chunk = ptr->get();  // not thread safe
    if (chunk == nullptr) {
        return nullptr;
    }
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;
    return &chunk->voxels[(y * CHUNK_D + lz) * CHUNK_W + lx];
}

const AABB* Chunks::isObstacleAt(float x, float y, float z) {
    int ix = floor(x);
    int iy = floor(y);
    int iz = floor(z);
    voxel* v = get(ix, iy, iz);
    if (v == nullptr) {
        if (iy >= CHUNK_H) {
            return nullptr;
        } else {
            static const AABB empty;
            return &empty;
        }
    }
    const auto& def = indices->blocks.require(v->id);
    if (def.obstacle) {
        glm::ivec3 offset {};
        if (v->state.segment) {
            glm::ivec3 point(ix, iy, iz);
            offset = seekOrigin(point, def, v->state) - point;
        }
        const auto& boxes =
            def.rotatable ? def.rt.hitboxes[v->state.rotation] : def.hitboxes;
        for (const auto& hitbox : boxes) {
            if (hitbox.contains(
                {x - ix - offset.x, y - iy - offset.y, z - iz - offset.z}
            )) {
                return &hitbox;
            }
        }
    }
    return nullptr;
}

bool Chunks::isSolidBlock(int32_t x, int32_t y, int32_t z) {
    voxel* v = get(x, y, z);
    if (v == nullptr) return false;
    return indices->blocks.get(v->id)->rt.solid;  //-V522
}

bool Chunks::isReplaceableBlock(int32_t x, int32_t y, int32_t z) {
    voxel* v = get(x, y, z);
    if (v == nullptr) return false;
    return indices->blocks.get(v->id)->replaceable;  //-V522
}

bool Chunks::isObstacleBlock(int32_t x, int32_t y, int32_t z) {
    voxel* v = get(x, y, z);
    if (v == nullptr) return false;
    return indices->blocks.get(v->id)->obstacle;  //-V522
}

ubyte Chunks::getLight(int32_t x, int32_t y, int32_t z, int channel) {
    if (y < 0 || y >= CHUNK_H) {
        return 0;
    }
    int cx = floordiv(x, CHUNK_W);
    int cz = floordiv(z, CHUNK_D);

    auto ptr = areaMap.getIf(cx, cz);
    if (ptr == nullptr) {
        return 0;
    }
    Chunk* chunk = ptr->get();
    if (chunk == nullptr) {
        return 0;
    }
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;
    return chunk->lightmap.get(lx, y, lz, channel);
}

light_t Chunks::getLight(int32_t x, int32_t y, int32_t z) {
    if (y < 0 || y >= CHUNK_H) {
        return 0;
    }
    int cx = floordiv(x, CHUNK_W);
    int cz = floordiv(z, CHUNK_D);

    auto ptr = areaMap.getIf(cx, cz);
    if (ptr == nullptr) {
        return 0;
    }
    Chunk* chunk = ptr->get();
    if (chunk == nullptr) {
        return 0;
    }
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;
    return chunk->lightmap.get(lx, y, lz);
}

Chunk* Chunks::getChunkByVoxel(int32_t x, int32_t y, int32_t z) {
    if (y < 0 || y >= CHUNK_H) {
        return nullptr;
    }
    int cx = floordiv(x, CHUNK_W);
    int cz = floordiv(z, CHUNK_D);
    if (auto ptr = areaMap.getIf(cx, cz)) {
        return ptr->get();
    }
    return nullptr;
}

Chunk* Chunks::getChunk(int x, int z) {
    if (auto ptr = areaMap.getIf(x, z)) {
        return ptr->get();
    }
    return nullptr;
}

glm::ivec3 Chunks::seekOrigin(
    glm::ivec3 pos, const Block& def, blockstate state
) {
    const auto& rotation = def.rotations.variants[state.rotation];
    auto segment = state.segment;
    while (true) {
        if (!segment) {
            return pos;
        }
        if (segment & 1) pos -= rotation.axisX;
        if (segment & 2) pos -= rotation.axisY;
        if (segment & 4) pos -= rotation.axisZ;

        if (auto* voxel = get(pos.x, pos.y, pos.z)) {
            segment = voxel->state.segment;
        } else {
            return pos;
        }
    }
}

void Chunks::eraseSegments(
    const Block& def, blockstate state, int x, int y, int z
) {
    const auto& rotation = def.rotations.variants[state.rotation];
    for (int sy = 0; sy < def.size.y; sy++) {
        for (int sz = 0; sz < def.size.z; sz++) {
            for (int sx = 0; sx < def.size.x; sx++) {
                if ((sx | sy | sz) == 0) {
                    continue;
                }
                glm::ivec3 pos(x, y, z);
                pos += rotation.axisX * sx;
                pos += rotation.axisY * sy;
                pos += rotation.axisZ * sz;
                set(pos.x, pos.y, pos.z, 0, {});
            }
        }
    }
}

static constexpr uint8_t segment_to_int(int sx, int sy, int sz) {
    return ((sx > 0) | ((sy > 0) << 1) | ((sz > 0) << 2));
}

void Chunks::repairSegments(
    const Block& def, blockstate state, int x, int y, int z
) {
    const auto& rotation = def.rotations.variants[state.rotation];
    const auto id = def.rt.id;
    const auto size = def.size;
    for (int sy = 0; sy < size.y; sy++) {
        for (int sz = 0; sz < size.z; sz++) {
            for (int sx = 0; sx < size.x; sx++) {
                if ((sx | sy | sz) == 0) {
                    continue;
                }
                blockstate segState = state;
                segState.segment = segment_to_int(sx, sy, sz);

                glm::ivec3 pos(x, y, z);
                pos += rotation.axisX * sx;
                pos += rotation.axisY * sy;
                pos += rotation.axisZ * sz;
                set(pos.x, pos.y, pos.z, id, segState);
            }
        }
    }
}

bool Chunks::checkReplaceability(
    const Block& def, blockstate state, glm::ivec3 origin, blockid_t ignore
) {
    const auto& rotation = def.rotations.variants[state.rotation];
    const auto size = def.size;
    for (int sy = 0; sy < size.y; sy++) {
        for (int sz = 0; sz < size.z; sz++) {
            for (int sx = 0; sx < size.x; sx++) {
                auto pos = origin;
                pos += rotation.axisX * sx;
                pos += rotation.axisY * sy;
                pos += rotation.axisZ * sz;
                if (auto vox = get(pos.x, pos.y, pos.z)) {
                    auto& target = indices->blocks.require(vox->id);
                    if (!target.replaceable && vox->id != ignore) {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }
    }
    return true;
}

void Chunks::setRotationExtended(
    const Block& def, blockstate state, glm::ivec3 origin, uint8_t index
) {
    auto newstate = state;
    newstate.rotation = index;

    // unable to rotate block (cause: obstacles)
    if (!checkReplaceability(def, newstate, origin, def.rt.id)) {
        return;
    }

    const auto& rotation = def.rotations.variants[index];
    const auto size = def.size;
    std::vector<glm::ivec3> segmentBlocks;

    for (int sy = 0; sy < size.y; sy++) {
        for (int sz = 0; sz < size.z; sz++) {
            for (int sx = 0; sx < size.x; sx++) {
                auto pos = origin;
                pos += rotation.axisX * sx;
                pos += rotation.axisY * sy;
                pos += rotation.axisZ * sz;

                blockstate segState = newstate;
                segState.segment = segment_to_int(sx, sy, sz);

                auto vox = get(pos);
                // checked for nullptr by checkReplaceability
                if (vox->id != def.rt.id) {
                    set(pos.x, pos.y, pos.z, def.rt.id, segState);
                } else {
                    vox->state = segState;
                    auto chunk = getChunkByVoxel(pos.x, pos.y, pos.z);
                    assert(chunk != nullptr);
                    chunk->setModifiedAndUnsaved();
                    segmentBlocks.emplace_back(pos);
                }
            }
        }
    }
    const auto& prevRotation = def.rotations.variants[state.rotation];
    for (int sy = 0; sy < size.y; sy++) {
        for (int sz = 0; sz < size.z; sz++) {
            for (int sx = 0; sx < size.x; sx++) {
                auto pos = origin;
                pos += prevRotation.axisX * sx;
                pos += prevRotation.axisY * sy;
                pos += prevRotation.axisZ * sz;
                if (std::find(
                        segmentBlocks.begin(), segmentBlocks.end(), pos
                    ) == segmentBlocks.end()) {
                    set(pos.x, pos.y, pos.z, 0, {});
                }
            }
        }
    }
}

void Chunks::setRotation(int32_t x, int32_t y, int32_t z, uint8_t index) {
    if (index >= BlockRotProfile::MAX_COUNT) {
        return;
    }
    auto vox = get(x, y, z);
    if (vox == nullptr) {
        return;
    }
    auto& def = indices->blocks.require(vox->id);
    if (!def.rotatable || vox->state.rotation == index) {
        return;
    }
    if (def.rt.extended) {
        setRotationExtended(def, vox->state, {x, y, z}, index);
    } else {
        vox->state.rotation = index;
        auto chunk = getChunkByVoxel(x, y, z);
        assert(chunk != nullptr);
        chunk->setModifiedAndUnsaved();
    }
}

void Chunks::set(
    int32_t x, int32_t y, int32_t z, uint32_t id, blockstate state
) {
    if (y < 0 || y >= CHUNK_H) {
        return;
    }
    int cx = floordiv(x, CHUNK_W);
    int cz = floordiv(z, CHUNK_D);
    auto ptr = areaMap.getIf(cx, cz);
    if (ptr == nullptr) {
        return;
    }
    Chunk* chunk = ptr->get();
    if (chunk == nullptr) {
        return;
    }
    int lx = x - cx * CHUNK_W;
    int lz = z - cz * CHUNK_D;

    // block finalization
    voxel& vox = chunk->voxels[(y * CHUNK_D + lz) * CHUNK_W + lx];
    const auto& prevdef = indices->blocks.require(vox.id);
    if (prevdef.inventorySize == 0) {
        chunk->removeBlockInventory(lx, y, lz);
    }
    if (prevdef.rt.extended && !vox.state.segment) {
        eraseSegments(prevdef, vox.state, x, y, z);
    }

    // block initialization
    const auto& newdef = indices->blocks.require(id);
    vox.id = id;
    vox.state = state;
    chunk->setModifiedAndUnsaved();
    if (!state.segment && newdef.rt.extended) {
        repairSegments(newdef, state, x, y, z);
    }

    if (y < chunk->bottom)
        chunk->bottom = y;
    else if (y + 1 > chunk->top)
        chunk->top = y + 1;
    else if (id == 0)
        chunk->updateHeights();

    if (lx == 0 && (chunk = getChunk(cx - 1, cz))) {
        chunk->flags.modified = true;
    }
    if (lz == 0 && (chunk = getChunk(cx, cz - 1))) {
        chunk->flags.modified = true;
    }
    if (lx == CHUNK_W - 1 && (chunk = getChunk(cx, cz))) {
        chunk->flags.modified = true;
    }
    if (lz == CHUNK_D - 1 && (chunk = getChunk(cx, cz + 1))) {
        chunk->flags.modified = true;
    }
}

voxel* Chunks::rayCast(
    glm::vec3 start,
    glm::vec3 dir,
    float maxDist,
    glm::vec3& end,
    glm::ivec3& norm,
    glm::ivec3& iend,
    std::set<blockid_t> filter
) {
    float px = start.x;
    float py = start.y;
    float pz = start.z;

    float dx = dir.x;
    float dy = dir.y;
    float dz = dir.z;

    float t = 0.0f;
    int ix = std::floor(px);
    int iy = std::floor(py);
    int iz = std::floor(pz);

    int stepx = (dx > 0.0f) ? 1 : -1;
    int stepy = (dy > 0.0f) ? 1 : -1;
    int stepz = (dz > 0.0f) ? 1 : -1;

    constexpr float infinity = std::numeric_limits<float>::infinity();
    constexpr float epsilon = 1e-6f;  // 0.000001
    float txDelta = (std::fabs(dx) < epsilon) ? infinity : std::fabs(1.0f / dx);
    float tyDelta = (std::fabs(dy) < epsilon) ? infinity : std::fabs(1.0f / dy);
    float tzDelta = (std::fabs(dz) < epsilon) ? infinity : std::fabs(1.0f / dz);

    float xdist = (stepx > 0) ? (ix + 1 - px) : (px - ix);
    float ydist = (stepy > 0) ? (iy + 1 - py) : (py - iy);
    float zdist = (stepz > 0) ? (iz + 1 - pz) : (pz - iz);

    float txMax = (txDelta < infinity) ? txDelta * xdist : infinity;
    float tyMax = (tyDelta < infinity) ? tyDelta * ydist : infinity;
    float tzMax = (tzDelta < infinity) ? tzDelta * zdist : infinity;

    int steppedIndex = -1;

    while (t <= maxDist) {
        voxel* voxel = get(ix, iy, iz);
        if (voxel == nullptr) {
            return nullptr;
        }

        const auto& def = indices->blocks.require(voxel->id);
        if ((filter.empty() && def.selectable) ||
            (!filter.empty() && filter.find(def.rt.id) == filter.end())) {
            end.x = px + t * dx;
            end.y = py + t * dy;
            end.z = pz + t * dz;
            iend.x = ix;
            iend.y = iy;
            iend.z = iz;

            if (!def.rt.solid) {
                const std::vector<AABB>& hitboxes =
                    def.rotatable ? def.rt.hitboxes[voxel->state.rotation]
                                  : def.hitboxes;

                scalar_t distance = maxDist;
                Ray ray(start, dir);

                bool hit = false;

                glm::vec3 offset {};
                if (voxel->state.segment) {
                    offset = seekOrigin(iend, def, voxel->state) - iend;
                }

                for (auto box : hitboxes) {
                    box.a += offset;
                    box.b += offset;
                    scalar_t boxDistance;
                    glm::ivec3 boxNorm;
                    if (ray.intersectAABB(
                            iend, box, maxDist, boxNorm, boxDistance
                        ) > RayRelation::None &&
                        boxDistance < distance) {
                        hit = true;
                        distance = boxDistance;
                        norm = boxNorm;
                        end = start + (dir * glm::vec3(distance));
                    }
                }

                if (hit) return voxel;
            } else {
                iend.x = ix;
                iend.y = iy;
                iend.z = iz;

                norm.x = norm.y = norm.z = 0;
                if (steppedIndex == 0) norm.x = -stepx;
                if (steppedIndex == 1) norm.y = -stepy;
                if (steppedIndex == 2) norm.z = -stepz;
                return voxel;
            }
        }
        if (txMax < tyMax) {
            if (txMax < tzMax) {
                ix += stepx;
                t = txMax;
                txMax += txDelta;
                steppedIndex = 0;
            } else {
                iz += stepz;
                t = tzMax;
                tzMax += tzDelta;
                steppedIndex = 2;
            }
        } else {
            if (tyMax < tzMax) {
                iy += stepy;
                t = tyMax;
                tyMax += tyDelta;
                steppedIndex = 1;
            } else {
                iz += stepz;
                t = tzMax;
                tzMax += tzDelta;
                steppedIndex = 2;
            }
        }
    }
    iend.x = ix;
    iend.y = iy;
    iend.z = iz;

    end.x = px + t * dx;
    end.y = py + t * dy;
    end.z = pz + t * dz;
    norm.x = norm.y = norm.z = 0;
    return nullptr;
}

glm::vec3 Chunks::rayCastToObstacle(
    glm::vec3 start, glm::vec3 dir, float maxDist
) {
    const float px = start.x;
    const float py = start.y;
    const float pz = start.z;

    float dx = dir.x;
    float dy = dir.y;
    float dz = dir.z;

    float t = 0.0f;
    int ix = floor(px);
    int iy = floor(py);
    int iz = floor(pz);

    int stepx = (dx > 0.0f) ? 1 : -1;
    int stepy = (dy > 0.0f) ? 1 : -1;
    int stepz = (dz > 0.0f) ? 1 : -1;

    constexpr float infinity = std::numeric_limits<float>::infinity();
    constexpr float epsilon = 1e-6f;  // 0.000001
    float txDelta = (std::fabs(dx) < epsilon) ? infinity : std::fabs(1.0f / dx);
    float tyDelta = (std::fabs(dy) < epsilon) ? infinity : std::fabs(1.0f / dy);
    float tzDelta = (std::fabs(dz) < epsilon) ? infinity : std::fabs(1.0f / dz);

    float xdist = (stepx > 0) ? (ix + 1 - px) : (px - ix);
    float ydist = (stepy > 0) ? (iy + 1 - py) : (py - iy);
    float zdist = (stepz > 0) ? (iz + 1 - pz) : (pz - iz);

    float txMax = (txDelta < infinity) ? txDelta * xdist : infinity;
    float tyMax = (tyDelta < infinity) ? tyDelta * ydist : infinity;
    float tzMax = (tzDelta < infinity) ? tzDelta * zdist : infinity;

    while (t <= maxDist) {
        voxel* voxel = get(ix, iy, iz);
        if (voxel) {
            const auto& def = indices->blocks.require(voxel->id);
            if (def.obstacle) {
                if (!def.rt.solid) {
                    const std::vector<AABB>& hitboxes =
                        def.rotatable ? def.rt.hitboxes[voxel->state.rotation]
                                      : def.modelBoxes;

                    scalar_t distance;
                    glm::ivec3 norm;
                    Ray ray(start, dir);

                    glm::ivec3 offset {};
                    if (voxel->state.segment) {
                        offset = seekOrigin({ix, iy, iz}, def, voxel->state) -
                                 glm::ivec3(ix, iy, iz);
                    }

                    for (const auto& box : hitboxes) {
                        // norm is dummy now, can be inefficient
                        if (ray.intersectAABB(
                                glm::ivec3(ix, iy, iz) + offset,
                                box,
                                maxDist,
                                norm,
                                distance
                            ) > RayRelation::None) {
                            return start + (dir * glm::vec3(distance));
                        }
                    }
                } else {
                    return glm::vec3(px + t * dx, py + t * dy, pz + t * dz);
                }
            }
        }
        if (txMax < tyMax) {
            if (txMax < tzMax) {
                ix += stepx;
                t = txMax;
                txMax += txDelta;
            } else {
                iz += stepz;
                t = tzMax;
                tzMax += tzDelta;
            }
        } else {
            if (tyMax < tzMax) {
                iy += stepy;
                t = tyMax;
                tyMax += tyDelta;
            } else {
                iz += stepz;
                t = tzMax;
                tzMax += tzDelta;
            }
        }
    }
    return glm::vec3(px + maxDist * dx, py + maxDist * dy, pz + maxDist * dz);
}

void Chunks::setCenter(int32_t x, int32_t z) {
    areaMap.setCenter(floordiv(x, CHUNK_W), floordiv(z, CHUNK_D));
}

void Chunks::resize(uint32_t newW, uint32_t newD) {
    areaMap.resize(newW, newD);
}

bool Chunks::putChunk(const std::shared_ptr<Chunk>& chunk) {
    return areaMap.set(chunk->x, chunk->z, chunk);
}

void Chunks::saveAndClear() {
    areaMap.clear();
}

void Chunks::save(Chunk* chunk) {
    if (chunk != nullptr) {
        AABB aabb(
            glm::vec3(chunk->x * CHUNK_W, -INFINITY, chunk->z * CHUNK_D),
            glm::vec3(
                (chunk->x + 1) * CHUNK_W, INFINITY, (chunk->z + 1) * CHUNK_D
            )
        );
        auto entities = level->entities->getAllInside(aabb);
        auto root = dynamic::create_map();
        root->put("data", level->entities->serialize(entities));
        if (!entities.empty()) {
            level->entities->despawn(std::move(entities));
            chunk->flags.entities = true;
        }
        worldFiles->getRegions().put(
            chunk,
            chunk->flags.entities ? json::to_binary(root, true)
                                  : std::vector<ubyte>()
        );
    }
}

void Chunks::saveAll() {
    const auto& chunks = areaMap.getBuffer();
    for (size_t i = 0; i < areaMap.area(); i++) {
        if (auto& chunk = chunks[i]) {
            save(chunk.get());
        }
    }
}
