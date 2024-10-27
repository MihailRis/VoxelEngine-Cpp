#include "ModelBatch.hpp"

#include "graphics/core/Mesh.hpp"
#include "graphics/core/Model.hpp"
#include "graphics/core/Atlas.hpp"
#include "graphics/core/Texture.hpp"
#include "assets/Assets.hpp"
#include "window/Window.hpp"
#include "voxels/Chunks.hpp"
#include "lighting/Lightmap.hpp"
#include "settings.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

#include <algorithm>

/// xyz, uv, color, compressed lights
inline constexpr uint VERTEX_SIZE = 9;

static const vattr attrs[] = {
    {3}, {2}, {3}, {1}, {0}
};

inline constexpr glm::vec3 X(1, 0, 0);
inline constexpr glm::vec3 Y(0, 1, 0);
inline constexpr glm::vec3 Z(0, 0, 1);

struct DecomposedMat4 {
    glm::vec3 scale;
    glm::mat3 rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
};

static glm::mat4 extract_rotation(glm::mat4 matrix) {
    DecomposedMat4 decomposed = {};
    glm::quat rotation;
    glm::decompose(
        matrix, 
        decomposed.scale, 
        rotation, 
        decomposed.translation, 
        decomposed.skew, 
        decomposed.perspective
    );
    return glm::toMat3(rotation);
}

ModelBatch::ModelBatch(
    size_t capacity,
    Assets* assets,
    Chunks* chunks,
    const EngineSettings* settings
)
    : buffer(std::make_unique<float[]>(capacity * VERTEX_SIZE)),
      capacity(capacity),
      index(0),
      mesh(std::make_unique<Mesh>(buffer.get(), 0, attrs)),
      assets(assets),
      chunks(chunks),
      settings(settings) {
    const ubyte pixels[] = {
        255, 255, 255, 255,
    };
    ImageData image(ImageFormat::rgba8888, 1, 1, pixels);
    blank = Texture::from(&image);
}

ModelBatch::~ModelBatch() = default;

void ModelBatch::draw(const model::Mesh& mesh, const glm::mat4& matrix, 
                      const glm::mat3& rotation, glm::vec3 tint,
                      const texture_names_map* varTextures,
                      bool backlight) {
    glm::vec3 gpos = matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    gpos += lightsOffset;
    light_t light = chunks->getLight(
        std::floor(gpos.x), 
        std::floor(std::min(CHUNK_H-1.0f, gpos.y)), 
        std::floor(gpos.z));
    light_t minIntensity = backlight ? 1 : 0;
    glm::vec4 lights(
        glm::max(Lightmap::extract(light, 0), minIntensity) / 15.0f,
        glm::max(Lightmap::extract(light, 1), minIntensity) / 15.0f,
        glm::max(Lightmap::extract(light, 2), minIntensity) / 15.0f,
        glm::max(Lightmap::extract(light, 3), minIntensity) / 15.0f
    );
    setTexture(mesh.texture, varTextures);
    size_t vcount = mesh.vertices.size();
    const auto& vertexData = mesh.vertices.data();
    for (size_t i = 0; i < vcount / 3; i++) {
        if (index + VERTEX_SIZE * 3 > capacity * VERTEX_SIZE) {
            flush();
        }
        for (size_t j = 0; j < 3; j++) {
            const auto vert = vertexData[i * 3 + j];
            auto norm = rotation * vert.normal;
            float d = glm::dot(norm, SUN_VECTOR);
            d = 0.8f + d * 0.2f;
            vertex(matrix * glm::vec4(vert.coord, 1.0f), vert.uv, lights*d, tint);
        }
    }
}

void ModelBatch::draw(glm::mat4 matrix,
                      glm::vec3 tint,
                      const model::Model* model,
                      const texture_names_map* varTextures) {
    for (const auto& mesh : model->meshes) {
        entries.push_back({
            matrix, extract_rotation(matrix), tint, &mesh, varTextures
        });
    }
}

void ModelBatch::render() {
    std::sort(entries.begin(), entries.end(), 
        [](const DrawEntry& a, const DrawEntry& b) {
            return a.mesh->texture < b.mesh->texture;
        }
    );
    bool backlight = settings->graphics.backlight.get();
    for (auto& entry : entries) {
        draw(
            *entry.mesh,
            entry.matrix,
            entry.rotation,
            entry.tint,
            entry.varTextures,
            backlight
        );
    }
    flush();
    entries.clear();
}

void ModelBatch::setLightsOffset(const glm::vec3& offset) {
    lightsOffset = offset;
}

void ModelBatch::setTexture(const std::string& name,
                            const texture_names_map* varTextures) {
    if (name.at(0) == '$') {
        const auto& found = varTextures->find(name);
        if (found == varTextures->end()) {
            return setTexture(nullptr);
        } else {
            return setTexture(found->second, varTextures);
        }
    }
    size_t sep = name.find(':');
    if (sep == std::string::npos) {
        setTexture(assets->get<Texture>(name));
    } else {
        auto atlas = assets->get<Atlas>(name.substr(0, sep));
        if (atlas == nullptr) {
            setTexture(nullptr);
        } else {
            setTexture(atlas->getTexture());
            if (auto reg = atlas->getIf(name.substr(sep+1))) {
                region = *reg;
            } else {
                setTexture("blocks:notfound", varTextures);
            }
        }
    }
}

void ModelBatch::setTexture(Texture* texture) {
    if (texture == nullptr) {
        texture = blank.get();
    }
    if (texture != this->texture) {
        flush();
    }
    this->texture = texture;
    region = UVRegion {0.0f, 0.0f, 1.0f, 1.0f};
}

void ModelBatch::flush() {
    if (index == 0) {
        return;
    }
    if (texture == nullptr) {
        texture = blank.get();
    }
    texture->bind();
    mesh->reload(buffer.get(), index / VERTEX_SIZE);
    mesh->draw();
    index = 0;
}
