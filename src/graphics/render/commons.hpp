#pragma once

#include <vector>
#include <memory>
#include <glm/vec3.hpp>

#include "graphics/core/MeshData.hpp"
#include "util/Buffer.hpp"

class Mesh;

struct SortingMeshEntry {
    glm::vec3 position;
    util::Buffer<float> vertexData;
    float distance;

    inline bool operator<(const SortingMeshEntry& o) const noexcept {
        return distance > o.distance;
    }
};

struct SortingMeshData {
    std::vector<SortingMeshEntry> entries;
};

struct ChunkMeshData {
    MeshData mesh;
    SortingMeshData sortingMesh;
};

struct ChunkMesh {
    std::shared_ptr<Mesh> mesh;
    SortingMeshData sortingMesh;
};
