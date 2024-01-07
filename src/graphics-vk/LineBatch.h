//
// Created by chelovek on 11/29/23.
//

#ifndef LINEBATCH_H
#define LINEBATCH_H
#include "Mesh.h"

namespace vulkan {

    class LineBatch {
        Mesh<VertexLine> *m_mesh = nullptr;
        VertexLine *m_buffer = nullptr;
        size_t m_index = 0;
        size_t m_capacity = 0;
        size_t m_vertexOffset = 0;
    public:
        LineBatch(size_t capacity = 4096);
        ~LineBatch();

        void line(float x1, float y1, float z1, float x2, float y2, float z2,
            float r, float g, float b, float a);
        void box(float x, float y, float z, float w, float h, float d,
                float r, float g, float b, float a);

        void begin();

        inline void line(const glm::vec3 a, const glm::vec3 b, const glm::vec4 color) {
            line(a.x, a.y, a.z, b.x, b.y, b.z, color.r, color.g, color.b, color.a);
        }

        inline void box(glm::vec3 xyz, glm::vec3 whd, glm::vec4 rgba) {
            box(xyz.x, xyz.y, xyz.z, whd.x, whd.y, whd.z,
                       rgba.r, rgba.g, rgba.b, rgba.a);
        }

        void render();

        void end();

        void lineWidth(float width);
    };

} // vulkan

#endif //LINEBATCH_H
