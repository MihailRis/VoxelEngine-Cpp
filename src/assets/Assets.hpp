#ifndef ASSETS_ASSETS_HPP_
#define ASSETS_ASSETS_HPP_

#include "../graphics/core/TextureAnimation.hpp"

#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <vector>

class Assets;

namespace audio {
    class Sound;
}

namespace model {
    struct Model;
}

namespace assetload {
    /// @brief final work to do in the main thread
    using postfunc = std::function<void(Assets*)>;
}

class Assets {
    std::vector<TextureAnimation> animations;

    using assets_map = std::unordered_map<std::string, std::shared_ptr<void>>;
    std::unordered_map<std::type_index, assets_map> assets;
public:
    Assets() {}
    Assets(const Assets&) = delete;
    ~Assets();

    const std::vector<TextureAnimation>& getAnimations();
    void store(const TextureAnimation& animation);

    template<class T>
    void store(std::unique_ptr<T> asset, const std::string& name) {
        assets[typeid(T)][name].reset(asset.release());
    }

    template<class T>
    T* get(const std::string& name) const {
        const auto& mapIter = assets.find(typeid(T));
        if (mapIter == assets.end()) {
            return nullptr;
        }
        const auto& map = mapIter->second;
        const auto& found = map.find(name);
        if (found == map.end()) {
            return nullptr;
        }
        return static_cast<T*>(found->second.get());
    }
};

#endif // ASSETS_ASSETS_HPP_
