//
// Created by chelovek on 12/11/23.
//

#ifndef BATCH3D_H
#define BATCH3D_H

#include "Mesh.h"

class UVRegion;
class ITexture;

namespace vulkan {

    class Batch3D {
        Vertex3DUI *m_buffer = nullptr;
        Mesh<Vertex3DUI> *m_mesh = nullptr;
        size_t m_capacity = 0;
        size_t m_index = 0;
        size_t m_currentOffset = 0;

        ITexture *m_blank = nullptr;
        ITexture *m_texture = nullptr;

        void vertex(float x, float y, float z,
                float u, float v,
                float r, float g, float b, float a);
        void vertex(glm::vec3 coord,
                float u, float v,
                float r, float g, float b, float a);
        void vertex(glm::vec3 point, glm::vec2 uvpoint,
                    float r, float g, float b, float a);

        void face(const glm::vec3& coord, float w, float h,
            const glm::vec3& axisX,
            const glm::vec3& axisY,
            const UVRegion& region,
            const glm::vec4& tint);
    public:
        Batch3D(size_t capacity);
        ~Batch3D();

        void begin();
        void texture(ITexture* texture);
        void sprite(glm::vec3 pos, glm::vec3 up, glm::vec3 right, float w, float h, const UVRegion& uv, glm::vec4 color);
        void xSprite(float w, float h, const UVRegion& uv, const glm::vec4 tint, bool shading=true);
        void blockCube(const glm::vec3 size, const UVRegion(&texfaces)[6], const glm::vec4 tint, bool shading=true);
        void flush();

        void end();
    };

} // vulkan

#endif //BATCH3D_H
