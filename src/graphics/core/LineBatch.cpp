#include "LineBatch.hpp"
#include "Mesh.hpp"

#include <GL/glew.h>

inline constexpr uint LB_VERTEX_SIZE = (3+4);

LineBatch::LineBatch(size_t capacity) : capacity(capacity) {
    const vattr attrs[] = { {3},{4}, {0} };
    buffer = std::make_unique<float[]>(capacity * LB_VERTEX_SIZE * 2);
    mesh = std::make_unique<Mesh>(buffer.get(), 0, attrs);
    index = 0;
}

LineBatch::~LineBatch(){
}

void LineBatch::line(
    float x1, float y1, 
    float z1, float x2, 
    float y2, float z2,
    float r, float g, float b, float a
) {
    if (index + LB_VERTEX_SIZE * 2 >= capacity) {
        render();
    }
    buffer[index] = x1;
    buffer[index+1] = y1;
    buffer[index+2] = z1;
    buffer[index+3] = r;
    buffer[index+4] = g;
    buffer[index+5] = b;
    buffer[index+6] = a;
    index += LB_VERTEX_SIZE;

    buffer[index] = x2;
    buffer[index+1] = y2;
    buffer[index+2] = z2;
    buffer[index+3] = r;
    buffer[index+4] = g;
    buffer[index+5] = b;
    buffer[index+6] = a;
    index += LB_VERTEX_SIZE;
}

void LineBatch::box(float x, float y, float z, float w, float h, float d,
        float r, float g, float b, float a) {
    w *= 0.5f;
    h *= 0.5f;
    d *= 0.5f;

    line(x-w, y-h, z-d, x+w, y-h, z-d, r,g,b,a);
    line(x-w, y+h, z-d, x+w, y+h, z-d, r,g,b,a);
    line(x-w, y-h, z+d, x+w, y-h, z+d, r,g,b,a);
    line(x-w, y+h, z+d, x+w, y+h, z+d, r,g,b,a);

    line(x-w, y-h, z-d, x-w, y+h, z-d, r,g,b,a);
    line(x+w, y-h, z-d, x+w, y+h, z-d, r,g,b,a);
    line(x-w, y-h, z+d, x-w, y+h, z+d, r,g,b,a);
    line(x+w, y-h, z+d, x+w, y+h, z+d, r,g,b,a);

    line(x-w, y-h, z-d, x-w, y-h, z+d, r,g,b,a);
    line(x+w, y-h, z-d, x+w, y-h, z+d, r,g,b,a);
    line(x-w, y+h, z-d, x-w, y+h, z+d, r,g,b,a);
    line(x+w, y+h, z-d, x+w, y+h, z+d, r,g,b,a);
}

void LineBatch::render(){
    if (index == 0)
        return;
    mesh->reload(buffer.get(), index / LB_VERTEX_SIZE);
    mesh->draw(GL_LINES);
    index = 0;
}

void LineBatch::lineWidth(float width) {
    glLineWidth(width);
}
