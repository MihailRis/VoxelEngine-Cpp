#pragma once

#include <vector>

#include "typedefs.hpp"
#include "util/Buffer.hpp"

/// @brief Vertex attribute info
struct VertexAttribute {
    ubyte size;
};

/// @brief Raw mesh data structure
struct MeshData {
    util::Buffer<float> vertices;
    util::Buffer<int> indices;
    util::Buffer<VertexAttribute> attrs;

    MeshData() = default;

    /// @param vertices vertex data buffer
    /// @param indices nullable indices buffer
    /// @param attrs vertex attribute sizes (must be null-terminated) 
    MeshData(
        util::Buffer<float> vertices, 
        util::Buffer<int> indices,
        util::Buffer<VertexAttribute> attrs
    ) : vertices(std::move(vertices)),
        indices(std::move(indices)),
        attrs(std::move(attrs)) {}
};
