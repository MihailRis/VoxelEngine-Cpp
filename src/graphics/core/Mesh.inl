#pragma once

#include "MeshData.hpp"

template<typename VertexStructure>
Mesh<VertexStructure>::Mesh(const MeshData<VertexStructure>& data)
 : Mesh(data.vertices.data(),
        data.vertices.size(),
        data.indices.data(),
        data.indices.size(),
        data.attrs.data()) {}

template<typename VertexStructure>
Mesh<VertexStructure>::Mesh(const VertexStructure* vertexBuffer, size_t vertices, const uint32_t* indexBuffer, size_t indices, const VertexAttribute* attrs) :
    ibo(0),
    vao(0),
    vbo(0),
    vertexCount(0),
    indexCount(0)
{
    MeshStats::meshesCount++;
    vertexSize = 0;
    for (int i = 0; attrs[i].count; i++) {
        vertexSize += attrs[i].size();
    }
    if(vertexSize!=sizeof(VertexStructure)){
        throw std::runtime_error("Vertex size mismatch!");
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    reload(vertexBuffer, vertices, indexBuffer, indices);

    // attributes
    int offset = 0;
    for (int i = 0; attrs[i].count; i++) {
        const VertexAttribute &attr = attrs[i];
        glVertexAttribPointer(i, attr.count, attr.type, attr.normalized, sizeof(VertexStructure), (GLvoid*)(size_t)offset);
        glEnableVertexAttribArray(i);
        offset += attr.size();
    }

    glBindVertexArray(0);
}
template<typename VertexStructure>
Mesh<VertexStructure>::~Mesh(){
    MeshStats::meshesCount--;
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    if (ibo != 0) glDeleteBuffers(1, &ibo);
}
template<typename VertexStructure>
void Mesh<VertexStructure>::reload(const VertexStructure *vertexBuffer, size_t vertexCount, const uint32_t *indexBuffer, size_t indexCount) {
    this->vertexCount = vertexCount;
    this->indexCount = indexCount;
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    if (vertexBuffer != nullptr && vertexCount != 0) {
        glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, vertexBuffer, GL_STREAM_DRAW);
    } else {
        glBufferData(GL_ARRAY_BUFFER, 0, {}, GL_STREAM_DRAW);
    }

    if (indexBuffer != nullptr && indexCount != 0) {
        if (ibo == 0)
            glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indexCount, indexBuffer, GL_STATIC_DRAW);
    } else if (ibo != 0) {
        glDeleteBuffers(1, &ibo);
    }

}

template<typename VertexStructure>
void Mesh<VertexStructure>::draw(unsigned int primitive) const {
    MeshStats::drawCalls++;
    glBindVertexArray(vao);
    if (ibo != 0) {
        glDrawElements(primitive, indexCount, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(primitive, 0, vertexCount);
    }
    glBindVertexArray(0);
}

template<typename VertexStructure>
void Mesh<VertexStructure>::draw() const {
    draw(GL_TRIANGLES);
}
