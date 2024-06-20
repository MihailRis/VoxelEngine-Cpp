#ifndef GRAPHICS_RENDER_MODEL_BATCH_HPP_
#define GRAPHICS_RENDER_MODEL_BATCH_HPP_

#include <memory>
#include <vector>
#include <glm/glm.hpp>

class Mesh;
class Texture;
class Chunks;

class ModelBatch {
    std::unique_ptr<float[]> buffer;
    size_t capacity;
    size_t index;

    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Texture> blank;

    glm::mat4 combined;
    std::vector<glm::mat4> matrices;
    glm::mat3 rotation;

    Chunks* chunks;

    static inline glm::vec3 SUN_VECTOR {0.411934f, 0.863868f, -0.279161f};

    inline void vertex(
        glm::vec3 pos, glm::vec2 uv, glm::vec4 light
    ) {
        float* buffer = this->buffer.get();
        pos = combined * glm::vec4(pos, 1.0f);
        buffer[index++] = pos.x;
        buffer[index++] = pos.y;
        buffer[index++] = pos.z;
        buffer[index++] = uv.x;
        buffer[index++] = uv.y;

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

    inline void plane(glm::vec3 pos, glm::vec3 right, glm::vec3 up, glm::vec3 norm, glm::vec4 lights) {
        norm = rotation * norm;
        float d = glm::dot(norm, SUN_VECTOR);
        d = 0.8f + d * 0.2f;
        
        auto color = lights * d;

        vertex(pos-right-up, {0,0}, color);
        vertex(pos+right-up, {1,0}, color);
        vertex(pos+right+up, {1,1}, color);

        vertex(pos-right-up, {0,0}, color);
        vertex(pos+right+up, {1,1}, color);
        vertex(pos-right+up, {0,1}, color);
    }
public:
    ModelBatch(size_t capacity, Chunks* chunks);
    ~ModelBatch();

    void pushMatrix(glm::mat4 matrix);
    void popMatrix();

    void box(glm::vec3 pos, glm::vec3 size, glm::vec4 lights);

    void test(glm::vec3 pos, glm::vec3 size);
    void flush();
};

#endif // GRAPHICS_RENDER_MODEL_BATCH_HPP_
