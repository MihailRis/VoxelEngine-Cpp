#ifndef FRONTEND_GRAPHICS_SKYBOX_H_
#define FRONTEND_GRAPHICS_SKYBOX_H_

#include "../../typedefs.h"

class Mesh;
class Shader;

class Skybox {
    uint fbo;
    uint cubemap;
    uint size;
    Mesh* mesh;
    Shader* shader;
    bool ready = false;
public:
    Skybox(uint size, Shader* shader);
    ~Skybox();

    void draw(Shader* shader);

    void refresh(float t, float mie, uint quality);
    void bind() const;
    void unbind() const;
    bool isReady() const {
        return ready;
    }
};

#endif // FRONTEND_GRAPHICS_SKYBOX_H_