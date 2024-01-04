//
// Created by chelovek on 12/11/23.
//

#include "Batch3D.h"

#include "texture/Image2d.h"
#include "../graphics/UVRegion.h"

namespace vulkan {
    void Batch3D::vertex(float x, float y, float z, float u, float v, float r, float g, float b, float a) {
        m_buffer[m_index++] = Vertex3DUI{glm::vec3(x, y, z), {u, v}, {r, g, b, a}};
    }

    void Batch3D::vertex(glm::vec3 coord, float u, float v, float r, float g, float b, float a) {
        m_buffer[m_index++] = Vertex3DUI{coord, {u, v}, {r, g, b, a}};
    }

    void Batch3D::vertex(glm::vec3 point, glm::vec2 uvpoint, float r, float g, float b, float a) {
        m_buffer[m_index++] = Vertex3DUI{point, uvpoint, {r, g, b, a}};
    }

    void Batch3D::face(const glm::vec3& coord, float w, float h,
                       const glm::vec3& axisX,
                       const glm::vec3& axisY,
                       const UVRegion& region,
                       const glm::vec4& tint) {
        vertex(coord, region.u1, region.v1,
           tint.r, tint.g, tint.b, tint.a);
        vertex(coord + axisX * w, region.u2, region.v1,
                tint.r, tint.g, tint.b, tint.a);
        vertex(coord + axisX * w + axisY * h, region.u2, region.v2,
               tint.r, tint.g, tint.b, tint.a);

        vertex(coord, region.u1, region.v1,
               tint.r, tint.g, tint.b, tint.a);
        vertex(coord + axisX * w + axisY * h, region.u2, region.v2,
               tint.r, tint.g, tint.b, tint.a);
        vertex(coord + axisY * h, region.u1, region.v2,
               tint.r, tint.g, tint.b, tint.a);
    }

    Batch3D::Batch3D(size_t capacity) : m_capacity(capacity) {
        m_mesh = new Mesh<Vertex3DUI>(nullptr, capacity);
        m_mesh->mapVertex(&m_buffer);

        constexpr unsigned char pixels[] = {
            255, 255, 255, 255
        };

        m_blank = new Image2d(pixels, 1, 1, VK_FORMAT_R8G8B8A8_UNORM);
        m_texture = m_blank;
    }

    Batch3D::~Batch3D() {
        delete m_mesh;
        delete m_blank;
    }

    void Batch3D::begin() {
        m_texture->bind();
        m_mesh->bind();
        m_index = 0;
        m_currentOffset = 0;
    }

    void Batch3D::texture(ITexture* texture) {
        if (m_texture == texture)
            return;
        flush();
        if (texture == nullptr) {
            m_texture = m_blank;
            m_texture->bind();
            return;
        }
        m_texture = texture;
        m_texture->bind();
    }

    void Batch3D::sprite(glm::vec3 pos, glm::vec3 up, glm::vec3 right, float w, float h, const UVRegion& uv,
        glm::vec4 color) {

        if (m_index >= m_capacity) return;

        const float r = color.r;
        const float g = color.g;
        const float b = color.b;
        const float a = color.a;

        vertex(pos.x - right.x * w - up.x * h,
                    pos.y - right.y * w - up.y * h,
                    pos.z - right.z * w - up.z * h,
                    uv.u1, uv.v1,
                    r,g,b,a);

        vertex(pos.x + right.x * w + up.x * h,
                pos.y + right.y * w + up.y * h,
                pos.z + right.z * w + up.z * h,
                uv.u2, uv.v2,
                r,g,b,a);

        vertex(pos.x - right.x * w - up.x * h,
                pos.y + right.y * w + up.y * h,
                pos.z - right.z * w - up.z * h,
                uv.u1, uv.v2,
                r,g,b,a);

        vertex(pos.x - right.x * w - up.x * h,
                pos.y - right.y * w - up.y * h,
                pos.z - right.z * w - up.z * h,
                uv.u1, uv.v1,
                r,g,b,a);

        vertex(pos.x + right.x * w + up.x * h,
                pos.y - right.y * w - up.y * h,
                pos.z + right.z * w + up.z * h,
                uv.u2, uv.v1,
                r,g,b,a);

        vertex(pos.x + right.x * w + up.x * h,
                pos.y + right.y * w + up.y * h,
                pos.z + right.z * w + up.z * h,
                uv.u2, uv.v2,
                r,g,b,a);
    }

    inline glm::vec4 do_tint(float value) {
        return {value, value, value, 1.0f};
    }

    void Batch3D::xSprite(float w, float h, const UVRegion& uv, const glm::vec4 tint, bool shading) {
        if (m_index >= m_capacity) return;

        face(glm::vec3(-w * 0.25f, 0.0f, 0.0f - w * 0.25f), w, h, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), uv, (shading ? do_tint(1.0f)*tint : tint));
        face(glm::vec3(w * 0.25f, 0.0f, w * 0.5f - w * 0.25f), w, h, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), uv, (shading ? do_tint(0.9f)*tint : tint));
    }

    void Batch3D::blockCube(const glm::vec3 size, const UVRegion(& texfaces)[6], const glm::vec4 tint, bool shading) {
        if (m_index >= m_capacity) return;
        glm::vec3 coord = (1.0f - size);
        face(coord + glm::vec3(0.0f, 0.0f, 0.0f), size.x, size.y, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), texfaces[5], (shading ? do_tint(0.8)*tint : tint));
        face(coord + glm::vec3(size.x, 0.0f, -size.z), size.x, size.y, glm::vec3(-1, 0, 0), glm::vec3(0, 1, 0), texfaces[4], (shading ? do_tint(0.8f)*tint : tint));
        face(coord + glm::vec3(0.0f, size.y, 0.0f), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, -1), texfaces[3], (shading ? do_tint(1.0f)*tint : tint));
        face(coord + glm::vec3(0.0f, 0.0f, -size.z), size.x, size.z, glm::vec3(1, 0, 0), glm::vec3(0, 0, 1), texfaces[2], (shading ? do_tint(0.7f)*tint : tint));
        face(coord + glm::vec3(0.0f, 0.0f, -size.z), size.z, size.y, glm::vec3(0, 0, 1), glm::vec3(0, 1, 0), texfaces[0], (shading ? do_tint(0.9f)*tint : tint));
        face(coord + glm::vec3(size.x, 0.0f, 0.0f), size.z, size.y, glm::vec3(0, 0, -1), glm::vec3(0, 1, 0), texfaces[1], (shading ? do_tint(0.9f)*tint : tint));
    }

    void Batch3D::flush() {
        const VertexOffset offset = {m_currentOffset, m_index - m_currentOffset};
        if (offset.count == 0 || offset.offset >= m_capacity) return;
        end();
        m_mesh->bind();
        m_mesh->draw(offset);

        m_currentOffset = m_index;
    }

    void Batch3D::end() {
        m_mesh->reload(nullptr, m_capacity);
    }
} // vulkan