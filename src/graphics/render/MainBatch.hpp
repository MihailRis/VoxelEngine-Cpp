#pragma once

#include <memory>
#include <stdint.h>
#include <glm/glm.hpp>

#include "typedefs.hpp"
#include "maths/UVRegion.hpp"

class Mesh;
class Texture;

class MainBatch {
    std::unique_ptr<float[]> const buffer;
    size_t const capacity;
    size_t index;

    UVRegion region {0.0f, 0.0f, 1.0f, 1.0f};

    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Texture> blank;

    const Texture* texture = nullptr;
public:
    /// xyz, uv, color, compressed lights
    static inline constexpr uint VERTEX_SIZE = 9;

    MainBatch(size_t capacity);

    ~MainBatch();

    void prepare(int vertices);
    void setTexture(const Texture* texture);
    void setTexture(const Texture* texture, const UVRegion& region);
    void flush();

    inline void vertex(
        glm::vec3 pos, glm::vec2 uv, glm::vec4 light, glm::vec3 tint
    ) {
        float* buffer = this->buffer.get();
        buffer[index++] = pos.x;
        buffer[index++] = pos.y;
        buffer[index++] = pos.z;
        buffer[index++] = uv.x * region.getWidth() + region.u1;
        buffer[index++] = uv.y * region.getHeight() + region.v1;
        buffer[index++] = tint.x;
        buffer[index++] = tint.y;
        buffer[index++] = tint.z;

        union {
            float floating;
            uint32_t integer;
        } compressed;

        compressed.integer  = (static_cast<uint32_t>(light.r * 255) & 0xff) << 24;
        compressed.integer |= (static_cast<uint32_t>(light.g * 255) & 0xff) << 16;
        compressed.integer |= (static_cast<uint32_t>(light.b * 255) & 0xff) << 8;
        compressed.integer |= (static_cast<uint32_t>(light.a * 255) & 0xff);

        buffer[index++] = compressed.floating;
    }
};
