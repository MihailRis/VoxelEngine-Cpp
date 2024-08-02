#ifndef GRAPHICS_CORE_BATCH2D_HPP_
#define GRAPHICS_CORE_BATCH2D_HPP_

#include <memory>
#include <stdlib.h>
#include <glm/glm.hpp>

#include "commons.hpp"
#include "../../maths/UVRegion.hpp"

class Mesh;
class Texture;

class Batch2D : public Flushable {
    std::unique_ptr<float[]> buffer;
    size_t capacity;
    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Texture> blank;
    size_t index;
    glm::vec4 color;
    Texture* currentTexture;
    DrawPrimitive primitive = DrawPrimitive::triangle;
    UVRegion region {0.0f, 0.0f, 1.0f, 1.0f};

    void setPrimitive(DrawPrimitive primitive);

    void vertex(
        float x, float y,
        float u, float v,
        float r, float g, float b, float a
    );
    
    void vertex(
        const glm::vec2& point,
        const glm::vec2& uvpoint,
        float r, float g, float b, float a
    );

public:
    Batch2D(size_t capacity);
    ~Batch2D();

    void begin();
    void texture(Texture* texture);
    void untexture();
    void setRegion(UVRegion region);
    void sprite(float x, float y, float w, float h, const UVRegion& region,
        const glm::vec4& tint);
    void sprite(float x, float y, float w, float h, int atlasRes, int index,
        const glm::vec4& tint);
    void point(float x, float y, float r, float g, float b, float a);
    
    inline void setColor(const glm::vec4& color) {
        this->color = color;
    }
    inline glm::vec4 getColor() const {
        return color;
    }
    
    void line(
        float x1, float y1, 
        float x2, float y2, 
        float r, float g, float b, float a
    );

    void lineRect(float x, float y, float w, float h);

    void rect(
        float x, float y,
        float w, float h,
        float ox, float oy,
        float angle, UVRegion region,
        bool flippedX, bool flippedY,
        const glm::vec4& tint
    );

    void rect(float x, float y, float w, float h);

    void rect(
        float x, float y, float w, float h,
        float u, float v, float tx, float ty,
        float r, float g, float b, float a
    );

    void rect(
        float x, float y, float w, float h,
        float r0, float g0, float b0,
        float r1, float g1, float b1,
        float r2, float g2, float b2,
        float r3, float g3, float b3,
        float r4, float g4, float b4, int sh
    );

    void flush() override;

    void lineWidth(float width);
};

#endif // GRAPHICS_CORE_BATCH2D_HPP_
