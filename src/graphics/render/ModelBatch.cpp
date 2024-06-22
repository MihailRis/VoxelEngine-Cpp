#include "ModelBatch.hpp"

#include "../core/Mesh.hpp"
#include "../core/Model.hpp"
#include "../core/Texture.hpp"
#include "../../assets/Assets.hpp"
#include "../../window/Window.hpp"
#include "../../voxels/Chunks.hpp"
#include "../../lighting/Lightmap.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

/// xyz, uv, compressed rgba
inline constexpr uint VERTEX_SIZE = 6;

static const vattr attrs[] = {
    {3}, {2}, {1}, {0}
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

ModelBatch::ModelBatch(size_t capacity, Assets* assets, Chunks* chunks)
  : buffer(std::make_unique<float[]>(capacity * VERTEX_SIZE)),
    capacity(capacity),
    index(0),
    mesh(std::make_unique<Mesh>(buffer.get(), 0, attrs)),
    combined(1.0f),
    assets(assets),
    chunks(chunks)
{
    ubyte pixels[] = {
        255, 255, 255, 255,
    };
    blank = std::make_unique<Texture>(pixels, 1, 1, ImageFormat::rgba8888);
}

ModelBatch::~ModelBatch() {
}

void ModelBatch::draw(const model::Mesh& mesh, const glm::mat4& matrix, const glm::mat3& rotation) {
    glm::vec3 gpos = matrix * glm::vec4(glm::vec3(), 1.0f);
    light_t light = chunks->getLight(gpos.x, gpos.y, gpos.z);
    glm::vec4 lights (
        Lightmap::extract(light, 0) / 15.0f,
        Lightmap::extract(light, 1) / 15.0f,
        Lightmap::extract(light, 2) / 15.0f,
        Lightmap::extract(light, 3) / 15.0f
    );
    setTexture(assets->get<Texture>(mesh.texture));
    size_t vcount = mesh.vertices.size();
    const auto& vertexData = mesh.vertices.data();
    for (size_t i = 0; i < vcount / 3; i++) {
        if (index + VERTEX_SIZE * 3 > capacity * VERTEX_SIZE) {
            flush();
        }
        for (size_t j = 0; j < 3; j++) {
            const auto& vert = vertexData[i * 3 + j];
            auto norm = rotation * vert.normal;
            float d = glm::dot(norm, SUN_VECTOR);
            d = 0.8f + d * 0.2f;
            
            auto color = lights * d;
            vertex(matrix * glm::vec4(vert.coord, 1.0f), vert.uv, color);
        }
    }
}

void ModelBatch::draw(const model::Model* model) {
    for (const auto& mesh : model->meshes) {
        entries.push_back({combined, rotation, &mesh});
    }
}

void ModelBatch::render() {
    std::sort(entries.begin(), entries.end(), 
        [](const DrawEntry& a, const DrawEntry& b) {
            return a.mesh->texture < b.mesh->texture;
        }
    );
    for (auto& entry : entries) {
        draw(*entry.mesh, entry.matrix, entry.rotation);
    }
    flush();
    entries.clear();
}

void ModelBatch::box(glm::vec3 pos, glm::vec3 size, glm::vec4 lights) {
    if (index + 36 < capacity*VERTEX_SIZE) {
        flush();
    }
    plane(pos+Z*size, X*size, Y*size, Z, lights);
    plane(pos-Z*size, -X*size, Y*size, -Z, lights);

    plane(pos+Y*size, X*size, -Z*size, Y, lights);
    plane(pos-Y*size, X*size, Z*size, -Y, lights);

    plane(pos+X*size, -Z*size, Y*size, X, lights);
    plane(pos-X*size, Z*size, Y*size, -X, lights);
}

void ModelBatch::setTexture(Texture* texture) {
    if (texture == nullptr) {
        texture = blank.get();
    }
    if (texture != this->texture) {
        flush();
    }
    this->texture = texture;
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

void ModelBatch::translate(glm::vec3 vec) {
    pushMatrix(glm::translate(glm::mat4(1.0f), vec));
}

void ModelBatch::rotate(glm::vec3 axis, float angle) {
    pushMatrix(glm::rotate(glm::mat4(1.0f), angle, axis));
}

void ModelBatch::scale(glm::vec3 vec) {
    pushMatrix(glm::scale(glm::mat4(1.0f), vec));
}

void ModelBatch::pushMatrix(glm::mat4 matrix) {
    matrices.push_back(combined);
    combined = combined * matrix;
    rotation = extract_rotation(combined);
}

void ModelBatch::popMatrix() {
    combined = matrices[matrices.size()-1];
    matrices.erase(matrices.end()-1);
    rotation = extract_rotation(combined);
}
