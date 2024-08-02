#ifndef GRAPHICS_RENDER_MODEL_BATCH_HPP_
#define GRAPHICS_RENDER_MODEL_BATCH_HPP_

#include "../../maths/UVRegion.hpp"

#include <memory>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

class Mesh;
class Texture;
class Chunks;
class Assets;

namespace model {
    struct Mesh;
    struct Model;
}

using texture_names_map = std::unordered_map<std::string, std::string>;

class ModelBatch {
    std::unique_ptr<float[]> const buffer;
    size_t const capacity;
    size_t index;

    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Texture> blank;

    Assets* assets;
    Chunks* chunks;
    Texture* texture = nullptr;
    UVRegion region {0.0f, 0.0f, 1.0f, 1.0f};

    static inline glm::vec3 SUN_VECTOR {0.411934f, 0.863868f, -0.279161f};

    inline void vertex(
        const glm::vec3& pos,
        const glm::vec2& uv,
        const glm::vec4& light,
        const glm::vec3& tint
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

    void draw(const model::Mesh& mesh, 
              const glm::mat4& matrix, 
              const glm::mat3& rotation,
        const glm::vec3& tint,
              const texture_names_map* varTextures);
    void setTexture(const std::string& name,
                    const texture_names_map* varTextures);
    void setTexture(Texture* texture);
    void flush();

    struct DrawEntry {
        glm::mat4 matrix;
        glm::mat3 rotation;
        glm::vec3 tint;
        const model::Mesh* mesh;
        const texture_names_map* varTextures;
    };
    std::vector<DrawEntry> entries;
public:
    ModelBatch(size_t capacity, Assets* assets, Chunks* chunks);
    ~ModelBatch();

    void draw(
        const glm::mat4& matrix,
        const glm::vec3& tint,
              const model::Model* model,
              const texture_names_map* varTextures);
    void render();
};

#endif // GRAPHICS_RENDER_MODEL_BATCH_HPP_
