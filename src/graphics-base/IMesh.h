//
// Created by chelovek on 11/21/23.
//

#ifndef IMESH_H
#define IMESH_H

#include "VertexAttribute.h"

class IMesh {
public:
    virtual ~IMesh() = default;

    virtual void reload(const float* vertexBuffer, size_t vertices, const int* indexBuffer = nullptr, size_t indices = 0) = 0;
    virtual void draw(unsigned int primitive) = 0;
    virtual void draw() = 0;
};

IMesh *createMesh(const float *vertexBuffer, size_t vertices, const int* indexBuffer, size_t indices, const vattr* attrs);
IMesh *createMesh(const float* vertexBuffer, size_t vertices, const vattr* attrs);

#endif //IMESH_H
