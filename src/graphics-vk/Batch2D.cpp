//
// Created by chelovek on 11/25/23.
//

#include "Batch2D.h"

#include "texture/Image2d.h"

namespace vulkan {
    Batch2D::Batch2D(size_t capacity) : m_capacity(capacity) {
        constexpr vattr attrs[] = {
            {2}, {2}, {4}, {0}
        };

        m_buffer = new Vertex2D[capacity];

        m_mesh = new Mesh(m_buffer, 0, attrs);

        constexpr unsigned char pixels[] = {
            255, 255, 255, 255
        };

        m_blank = new Image2d(pixels, 1, 1, VK_FORMAT_R8G8B8A8_SRGB);
        m_texture = m_blank;
    }

    Batch2D::~Batch2D() {
        delete[] m_buffer;
        delete m_mesh;
        delete m_blank;
    }

    void Batch2D::texture(ITexture* texture) {
        if (texture == nullptr) {
            m_blank->bind();
            return;
        }
        m_texture = texture;
        m_texture->bind();
    }

    void Batch2D::begin() {
        m_index = 0;
    }

    void Batch2D::line(float x1, float y1, float x2, float y2, float r, float g, float b, float a) {
        if (m_index >= m_capacity) {
            return;
        }

        m_buffer[m_index++] = Vertex2D{{x1, y1}, {0, 0}, {r, g, b, a}};
        m_buffer[m_index++] = Vertex2D{{x2, y2}, {1, 1}, {r, g, b, a}};
    }

    void Batch2D::point(float x, float y, float r, float g, float b, float a){
        if (m_index >= m_capacity) {
            return;
        }

        m_buffer[m_index++] = Vertex2D{{x, y}, {0, 0}, {r,g,b,a}};
        render();
    }

    void Batch2D::rect(float x, float y, float w, float h, float ox, float oy, float angle, UVRegion region,
        bool flippedX, bool flippedY, glm::vec4 tint) {

        if (m_index >= m_capacity) {
            return;
        }

        float centerX = w*ox;
        float centerY = h*oy;
        float acenterX = w-centerX;
        float acenterY = h-centerY;

        float _x1 = -centerX;
        float _y1 = -centerY;

        float _x2 = -centerX;
        float _y2 = +acenterY;

        float _x3 = +acenterX;
        float _y3 = +acenterY;

        float _x4 = +acenterX;
        float _y4 = -centerY;

        float x1,y1,x2,y2,x3,y3,x4,y4;

        if (angle != 0) {
            float s = sin(angle);
            float c = cos(angle);

            x1 = c * _x1 - s * _y1;
            y1 = s * _x1 + c * _y1;

            x2 = c * _x2 - s * _y2;
            y2 = s * _x2 + c * _y2;

            x3 = c * _x3 - s * _y3;
            y3 = s * _x3 + c * _y3;

            x4 = x1 + (x3 - x2);
            y4 = y3 - (y2 - y1);
        } else {
            x1 = _x1;
            y1 = _y1;
            x2 = _x2;
            y2 = _y2;
            x3 = _x3;
            y3 = _y3;
            x4 = _x4;
            y4 = _y4;
        }

        x1 += x; x2 += x; x3 += x; x4 += x;
        y1 += y; y2 += y; y3 += y; y4 += y;

        float u1 = region.u1;
        float v1 = region.v1;
        float u2 = region.u1;
        float v2 = region.v2;
        float u3 = region.u2;
        float v3 = region.v2;
        float u4 = region.u2;
        float v4 = region.v1;

        if (flippedX) {
            float temp = u1;
            u1 = u3;
            u4 = temp;
            u2 = u3;
            u3 = temp;
        }
        if (flippedY) {
            float temp = v1;
            v1 = v2;
            v4 = v2;
            v2 = temp;
            v3 = temp;
        }

        m_buffer[m_index++] = Vertex2D{{x1, y1}, {u1, v1}, {tint.r, tint.g, tint.b, tint.a}};
        m_buffer[m_index++] = Vertex2D{{x2, y2}, {u2, v2}, {tint.r, tint.g, tint.b, tint.a}};
        m_buffer[m_index++] = Vertex2D{{x3, y3}, {u3, v3}, {tint.r, tint.g, tint.b, tint.a}};

        /* Right down triangle */
        m_buffer[m_index++] = Vertex2D{{x1, y1}, {u1, v1}, {tint.r, tint.g, tint.b, tint.a}};
        m_buffer[m_index++] = Vertex2D{{x3, y3}, {u3, v3}, {tint.r, tint.g, tint.b, tint.a}};
        m_buffer[m_index++] = Vertex2D{{x4, y4}, {u4, v4}, {tint.r, tint.g, tint.b, tint.a}};
    }

    void Batch2D::render() {
        m_mesh->reload(m_buffer, m_index);
        m_mesh->draw();
    }
} // vulkan