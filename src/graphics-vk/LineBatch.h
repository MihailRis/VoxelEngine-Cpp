//
// Created by chelovek on 11/29/23.
//

#ifndef LINEBATCH_H
#define LINEBATCH_H
#include "Mesh.h"

namespace vulkan {

    class LineBatch {
        Mesh<VertexLines> *m_mesh = nullptr;
        VertexLines *m_buffer = nullptr;
        size_t m_index = 0;
        size_t m_capacity = 0;
    public:
        LineBatch(size_t capacity);
        ~LineBatch();

        void line(float x1, float y1, float z1, float x2, float y2, float z2,
            float r, float g, float b, float a);
        void box(float x, float y, float z, float w, float h, float d,
                float r, float g, float b, float a);

        void render();
        void lineWidth(float width);
    };

} // vulkan

#endif //LINEBATCH_H
