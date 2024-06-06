#ifndef GRAPHICS_CORE_TEXTURE_ANIMATION_HPP_
#define GRAPHICS_CORE_TEXTURE_ANIMATION_HPP_

#include "../../typedefs.hpp"

#include <glm/glm.hpp>
#include <vector>

class Assets;
class Texture;
class Framebuffer;

constexpr float DEFAULT_FRAME_DURATION = 0.150f;

struct Frame {
    glm::ivec2 srcPos;
    glm::ivec2 dstPos;
    glm::ivec2 size;
    float duration = DEFAULT_FRAME_DURATION;
};

class TextureAnimation {
public:
    TextureAnimation(Texture* srcTex, Texture* dstTex) : srcTexture(srcTex), dstTexture(dstTex) {};
    ~TextureAnimation() = default;

    void addFrame(const Frame& frame) { frames.emplace_back(frame); };

    size_t currentFrame = 0;
    float timer = 0.f;
    Texture* srcTexture;
    Texture* dstTexture;
    std::vector<Frame> frames;
};

class TextureAnimator {
public:
    TextureAnimator();
    ~TextureAnimator();

    void addAnimation(const TextureAnimation& animation) { animations.emplace_back(animation); };
    void addAnimations(const std::vector<TextureAnimation>& animations);

    void update(float delta);
private:
    uint fboR;
    uint fboD;

    std::vector<TextureAnimation> animations;
};

#endif // GRAPHICS_CORE_TEXTURE_ANIMATION_HPP_
