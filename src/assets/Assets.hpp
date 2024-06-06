#ifndef ASSETS_ASSETS_HPP_
#define ASSETS_ASSETS_HPP_

#include "../graphics/core/TextureAnimation.hpp"

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <vector>

class Texture;
class Shader;
class Font;
class Atlas;
class Assets;
class UiDocument;

namespace audio {
    class Sound;
}

namespace assetload {
    /// @brief final work to do in the main thread
    using postfunc = std::function<void(Assets*)>;
}

class Assets {
    std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Font>> fonts;
    std::unordered_map<std::string, std::shared_ptr<Atlas>> atlases;
    std::unordered_map<std::string, std::shared_ptr<UiDocument>> layouts;
    std::unordered_map<std::string, std::shared_ptr<audio::Sound>> sounds;
    std::vector<TextureAnimation> animations;
public:
    Assets() {}
    Assets(const Assets&) = delete;
    ~Assets();
    
    Texture* getTexture(std::string name) const;
    void store(std::unique_ptr<Texture> texture, std::string name);

    Shader* getShader(std::string name) const;
    void store(std::unique_ptr<Shader> shader, std::string name);

    Font* getFont(std::string name) const;
    void store(std::unique_ptr<Font> font, std::string name);

    Atlas* getAtlas(std::string name) const;
    void store(std::unique_ptr<Atlas> atlas, std::string name);

    audio::Sound* getSound(std::string name) const;
    void store(std::unique_ptr<audio::Sound> sound, std::string name);

    const std::vector<TextureAnimation>& getAnimations();
    void store(const TextureAnimation& animation);

    UiDocument* getLayout(std::string name) const;
    void store(std::unique_ptr<UiDocument> layout, std::string name);
};

#endif // ASSETS_ASSETS_HPP_
