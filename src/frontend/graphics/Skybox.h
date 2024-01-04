#ifndef FRONTEND_GRAPHICS_SKYBOX_H_
#define FRONTEND_GRAPHICS_SKYBOX_H_

#include "../../typedefs.h"

class Mesh;
class IShader;

class Skybox {
    uint fbo;
    uint cubemap;
    uint size;
    Mesh* mesh;
    IShader* shader;
    bool ready = false;
public:
    Skybox(uint size, IShader* shader);
    ~Skybox();

    void draw(IShader* shader);

    void refresh(float t, float mie, uint quality);
    void bind() const;
    void unbind() const;
    bool isReady() const {
        return ready;
    }
};

#endif // FRONTEND_GRAPHICS_SKYBOX_H_