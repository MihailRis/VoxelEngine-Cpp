#pragma once

#include "MeshData.hpp"


struct MeshStats {
    static int meshesCount;
    static int drawCalls;
};


template<typename VertexStructure>
class Mesh {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    size_t vertexCount;
    size_t indexCount;
    size_t vertexSize;

public:
    explicit Mesh(const MeshData<VertexStructure> &data);

    Mesh(const VertexStructure *vertexBuffer, size_t vertices, const uint32_t *indexBuffer, size_t indices,
         const VertexAttribute *attrs);

    Mesh(const VertexStructure *vertexBuffer, size_t vertices, const VertexAttribute *attrs) : Mesh<VertexStructure>(
        vertexBuffer, vertices, nullptr, 0, attrs) {
    };

    ~Mesh();

    /// @brief Update GL vertex and index buffers data without changing VAO attributes
    /// @param vertexBuffer vertex data buffer
    /// @param vertexCount number of vertices in new buffer
    /// @param indexBuffer indices buffer
    /// @param indexCount number of values in indices buffer
    void reload(const VertexStructure *vertexBuffer, size_t vertexCount, const uint32_t *indexBuffer = nullptr,
                size_t indexCount = 0);

    /// @brief Draw mesh with specified primitives type
    /// @param primitive primitives type
    void draw(unsigned int primitive) const;

    /// @brief Draw mesh as triangles
    void draw() const;

    /// @brief Total numbers of alive mesh objects
};

#include "graphics/core/Mesh.inl"
