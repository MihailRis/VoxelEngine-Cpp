#ifndef GRAPHICS_RENDER_SKYBOX_H_
#define GRAPHICS_RENDER_SKYBOX_H_

#include <memory>
#include <string>
#include <vector>
#include "../../typedefs.h"
#include "../../maths/fastmaths.h"

class Mesh;
class Shader;
class Assets;
class Camera;
class Batch3D;
class Framebuffer;
class GfxContext;

struct skysprite {
    std::string texture;
    float phase;
    float distance;
    bool emissive;
};

class Skybox {
    std::unique_ptr<Framebuffer> fbo;
    uint size;
    Shader* shader;
    bool ready = false;
    FastRandom random;

    std::unique_ptr<Mesh> mesh;
    std::unique_ptr<Batch3D> batch3d;
    std::vector<skysprite> sprites;

    void drawStars(float angle, float opacity);
    void drawBackground(Camera* camera, Assets* assets, int width, int height);
public:
    Skybox(uint size, Shader* shader);
    ~Skybox();

    void draw(
        const GfxContext& pctx, 
        Camera* camera, 
        Assets* assets, 
        float daytime,
        float fog
    );

    void refresh(const GfxContext& pctx, float t, float mie, uint quality);
    void bind() const;
    void unbind() const;
    bool isReady() const {
        return ready;
    }
};

#endif // GRAPHICS_RENDER_SKYBOX_H_
