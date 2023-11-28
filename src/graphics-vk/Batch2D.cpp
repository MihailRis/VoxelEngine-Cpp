//
// Created by chelovek on 11/25/23.
//

#include "Batch2D.h"

#include "texture/Image2d.h"

namespace vulkan {
    Batch2D::Batch2D(size_t capacity) : m_capacity(capacity) {
        m_buffer = new Vertex2D[capacity];

        m_mesh = new Mesh(m_buffer, capacity);

        constexpr unsigned char pixels[] = {
            255, 255, 255, 255
        };

        m_blank = new Image2d(pixels, 1, 1, VK_FORMAT_R8G8B8A8_SRGB);
        m_texture = m_blank;
    }

    Batch2D::~Batch2D() {
        delete m_mesh;
        delete m_blank;
    }

    void Batch2D::setColor(const glm::vec4& color) {
        m_color = color;
    }

    const glm::vec4 &Batch2D::getColor() const {
        return m_color;
    }

    void Batch2D::texture(ITexture* texture) {
        if (m_texture == texture)
            return;
        render();
        if (texture == nullptr) {
            m_texture = m_blank;
            return;
        }
        m_texture = texture;
    }

    void Batch2D::begin() {
        m_texture->bind();
        m_index = 0;
        m_currentOffset = 0;
    }

    void Batch2D::sprite(float x, float y, float w, float h, const UVRegion& region, glm::vec4 tint) {
        rect(x, y, w, h, region.u1, region.v1, region.u2-region.u1, region.v2-region.v1, tint.r, tint.g, tint.b, tint.a);
    }

    void Batch2D::sprite(float x, float y, float w, float h, int atlasRes, int index, glm::vec4 tint) {
        const float scale = 1.0f / static_cast<float>(atlasRes);
        float u = (index % atlasRes) * scale;
        float v = 1.0f - ((index / atlasRes) * scale) - scale;
        rect(x, y, w, h, u, v, scale, scale, tint.r, tint.g, tint.b, tint.a);
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
    }

    void Batch2D::rect(float x, float y, float w, float h) {
        if (m_index >= m_capacity) {
            return;
        }

        const float r = m_color.r;
        const float g = m_color.g;
        const float b = m_color.b;
        const float a = m_color.a;

        m_buffer[m_index++] = Vertex2D{{x, y}, {0, 0}, {r,g,b,a}};
        m_buffer[m_index++] = Vertex2D{{x, y+h}, {0, 1}, {r,g,b,a}};
        m_buffer[m_index++] = Vertex2D{{x+w, y+h}, {1, 1}, {r,g,b,a}};

        m_buffer[m_index++] = Vertex2D{{x, y}, {0, 0}, {r,g,b,a}};
        m_buffer[m_index++] = Vertex2D{{x+w, y+h}, {1, 1}, {r,g,b,a}};
        m_buffer[m_index++] = Vertex2D{{x+w, y}, {1, 0}, {r,g,b,a}};
    }

    void Batch2D::rect(float x, float y, float w, float h, float u, float v, float tx, float ty, float r, float g,
        float b, float a) {
        m_buffer[m_index++] = Vertex2D{{x, y}, {u, v+ty}, {r,g,b,a}};
        m_buffer[m_index++] = Vertex2D{{x, y+h}, {u, v}, {r,g,b,a}};
        m_buffer[m_index++] = Vertex2D{{x+w, y+h}, {u+tx, v}, {r,g,b,a}};

        m_buffer[m_index++] = Vertex2D{{x, y}, {u, v+ty}, {r,g,b,a}};
        m_buffer[m_index++] = Vertex2D{{x+w, y+h}, {u+tx, v}, {r,g,b,a}};
        m_buffer[m_index++] = Vertex2D{{x+w, y}, {u+tx, v+ty}, {r,g,b,a}};
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
        const VertexOffset offset = {m_currentOffset, m_index - m_currentOffset};
        if (offset.count == 0 || offset.offset >= m_capacity) return;
        m_mesh->reload(m_buffer, m_index);
        m_texture->bind();
        m_mesh->draw(offset);

        m_currentOffset = m_index;
    }
} // vulkan