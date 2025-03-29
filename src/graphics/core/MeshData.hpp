#pragma once

#include <stdexcept>
#include <GL/glew.h>

#include "typedefs.hpp"
#include "util/Buffer.hpp"

/// @brief Vertex attribute info
struct VertexAttribute {
    uint32_t type;
    bool normalized;
    ubyte count;


    [[nodiscard]] uint32_t size() const {
        switch (type) {
            case GL_FLOAT:
                return count * sizeof(float);
            case GL_UNSIGNED_INT:
            case GL_INT:
                return count * sizeof(uint32_t);
            case GL_UNSIGNED_SHORT:
            case GL_SHORT:
                return count * sizeof(uint16_t);
            case GL_UNSIGNED_BYTE:
            case GL_BYTE:
                return count * sizeof(uint8_t);
            default:
                throw std::runtime_error("VertexAttribute type is not supported");
        }
    }
};

/// @brief Raw mesh data structure
template<typename VertexStructure>
struct MeshData {
    util::Buffer<VertexStructure> vertices;
    util::Buffer<uint32_t> indices;
    util::Buffer<VertexAttribute> attrs;

    MeshData() = default;

    /// @param vertices vertex data buffer
    /// @param indices nullable indices buffer
    /// @param attrs vertex attribute sizes (must be null-terminated) 
    MeshData(
        util::Buffer<VertexStructure> vertices,
        util::Buffer<uint32_t> indices,
        util::Buffer<VertexAttribute> attrs
    ) : vertices(std::move(vertices)),
        indices(std::move(indices)),
        attrs(std::move(attrs)) {}
};
