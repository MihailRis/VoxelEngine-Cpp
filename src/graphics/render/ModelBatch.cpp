#include "ModelBatch.hpp"

#include "../core/Mesh.hpp"
#include "../core/Texture.hpp"
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

ModelBatch::ModelBatch(size_t capacity, Chunks* chunks)
  : buffer(std::make_unique<float[]>(capacity * VERTEX_SIZE)),
    capacity(capacity),
    index(0),
    mesh(std::make_unique<Mesh>(buffer.get(), 0, attrs)),
    combined(1.0f),
    chunks(chunks)
{
    ubyte pixels[] = {
        255, 255, 255, 255,
    };
    blank = std::make_unique<Texture>(pixels, 1, 1, ImageFormat::rgba8888);
}

ModelBatch::~ModelBatch() {
}

void ModelBatch::test(glm::vec3 pos, glm::vec3 size) {
    float time = static_cast<float>(Window::time());
    pushMatrix(glm::translate(glm::mat4(1.0f), pos));
    pushMatrix(glm::rotate(glm::mat4(1.0f), glm::sin(time*7*0.1f), glm::vec3(0,1,0)));
    pushMatrix(glm::rotate(glm::mat4(1.0f), glm::sin(time*11*0.1f), glm::vec3(1,0,0)));
    pushMatrix(glm::rotate(glm::mat4(1.0f), glm::sin(time*17*0.1f), glm::vec3(0,0,1)));
    pushMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0, glm::sin(time*2), 0)));
    box({}, size);
    popMatrix();
    popMatrix();
    popMatrix();
    popMatrix();
    popMatrix();
}

void ModelBatch::box(glm::vec3 pos, glm::vec3 size) {
    if (index + 36 < capacity*VERTEX_SIZE) {
        flush();
    }
    glm::vec3 gpos = combined * glm::vec4(pos, 1.0f);
    light_t light = chunks->getLight(gpos.x, gpos.y, gpos.z);
    glm::vec4 lights (
        Lightmap::extract(light, 0) / 15.0f,
        Lightmap::extract(light, 1) / 15.0f,
        Lightmap::extract(light, 2) / 15.0f,
        Lightmap::extract(light, 3) / 15.0f
    );

    plane(pos+Z, X*size, Y*size, Z, lights);
    plane(pos-Z, -X*size, Y*size, -Z, lights);

    plane(pos+Y, X*size, -Z*size, Y, lights);
    plane(pos-Y, X*size, Z*size, -Y, lights);

    plane(pos+X, -Z*size, Y*size, X, lights);
    plane(pos-X, Z*size, Y*size, -X, lights);
}

void ModelBatch::flush() {
    if (index == 0) {
        return;
    }
    // blank->bind();
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
