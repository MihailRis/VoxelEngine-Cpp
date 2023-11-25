//
// Created by chelovek on 11/25/23.
//

#ifndef BATCH2D_H
#define BATCH2D_H

#include "Mesh.h"
#include "Vertices.h"
#include "../graphics/UVRegion.h"

class ITexture;

namespace vulkan {

    class Batch2D {
        Vertex2D *m_buffer = nullptr;
        Mesh<Vertex2D> *m_mesh = nullptr;
        size_t m_capacity = 0;
        size_t m_index = 0;

        ITexture *m_blank = nullptr;
        ITexture *m_texture = nullptr;
    public:
        Batch2D(size_t capacity);
        ~Batch2D();

        void texture(ITexture* texture);

        void begin();

        void line(float x1, float y1, float x2, float y2, float r, float g, float b, float a);

        void point(float x, float y, float r, float g, float b, float a);

        void rect(float x, float y,
                  float w, float h,
                  float ox, float oy,
                  float angle, UVRegion region,
                  bool flippedX, bool flippedY,
                  glm::vec4 tint);

        void render();

    };

} // vulkan

#endif //BATCH2D_H
