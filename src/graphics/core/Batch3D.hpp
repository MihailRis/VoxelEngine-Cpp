#pragma once

#include "typedefs.hpp"
#include "commons.hpp"

#include <memory>
#include <stdlib.h>
#include <glm/glm.hpp>

class Mesh;
class Texture;
struct UVRegion;

class Batch3D : public Flushable {
    std::unique_ptr<float[]> buffer;
    size_t capacity;
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Texture> blank;
    size_t index;
    glm::vec4 tint {1.0f};
    
    const Texture* currentTexture;

    void vertex(
        float x, float y, float z,
        float u, float v,
        float r, float g, float b, float a
    );
    void vertex(
        glm::vec3 coord,
        float u, float v,
        float r, float g, float b, float a
    );
    void vertex(
        glm::vec3 point, glm::vec2 uvpoint,
        float r, float g, float b, float a
    );
    void face(
        const glm::vec3& coord, float w, float h,
        const glm::vec3& axisX,
        const glm::vec3& axisY,
        const UVRegion& region,
        const glm::vec4& tint
    );

public:
    Batch3D(size_t capacity);
    ~Batch3D();

    void begin();
    void texture(const Texture* texture);
    void sprite(
        const glm::vec3& pos,
        const glm::vec3& up,
        const glm::vec3& right,
        float w,
        float h,
        const UVRegion& uv,
        const glm::vec4& tint
    );
    void sprite(
        const glm::vec3& pos,
        const glm::vec3& up,
        const glm::vec3& right,
        float w,
        float h,
        int atlasRes,
        int index,
        const glm::vec4& tint
    );
    void xSprite(
        float w,
        float h,
        const UVRegion& uv,
        const glm::vec4& tint,
        bool shading = true
    );
    void cube(
        const glm::vec3& coords,
        const glm::vec3& size,
        const UVRegion (&texfaces)[6],
        const glm::vec4& tint,
        bool shading = true
    );
    void blockCube(
        const glm::vec3& size,
        const UVRegion (&texfaces)[6],
        const glm::vec4& tint,
        bool shading = true
    );
    void vertex(const glm::vec3& pos, const glm::vec2& uv, const glm::vec4& tint);
    void point(const glm::vec3& pos, const glm::vec4& tint);
    void flush() override;
    void flushPoints();

    void setColor(const glm::vec4& color);
    const glm::vec4& getColor() const;
};
