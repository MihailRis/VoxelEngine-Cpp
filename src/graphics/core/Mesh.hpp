#pragma once

#include <stdlib.h>

#include "typedefs.hpp"
#include "MeshData.hpp"

class Mesh {
    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    size_t vertices;
    size_t indices;
    size_t vertexSize;
public:
    Mesh(const MeshData& data);
    Mesh(const float* vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices, const VertexAttribute* attrs);
    Mesh(const float* vertexBuffer, size_t vertices, const VertexAttribute* attrs) :
        Mesh(vertexBuffer, vertices, nullptr, 0, attrs) {};
    ~Mesh();

    /// @brief Update GL vertex and index buffers data without changing VAO attributes
    /// @param vertexBuffer vertex data buffer
    /// @param vertices number of vertices in new buffer
    /// @param indexBuffer indices buffer
    /// @param indices number of values in indices buffer
    void reload(const float* vertexBuffer, size_t vertices, const int* indexBuffer = nullptr, size_t indices = 0);
    
    /// @brief Draw mesh with specified primitives type
    /// @param primitive primitives type
    void draw(unsigned int primitive) const;

    /// @brief Draw mesh as triangles
    void draw() const;

    /// @brief Total numbers of alive mesh objects
    static int meshesCount;
    static int drawCalls;
};
