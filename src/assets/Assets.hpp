#ifndef ASSETS_ASSETS_HPP_
#define ASSETS_ASSETS_HPP_

#include "../graphics/core/TextureAnimation.hpp"

#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <vector>

class Assets;

enum class AssetType {
    TEXTURE,
    SHADER,
    FONT,
    ATLAS,
    LAYOUT,
    SOUND,
    MODEL
};

namespace assetload {
    /// @brief final work to do in the main thread
    using postfunc = std::function<void(Assets*)>;

    using setupfunc = std::function<void(const Assets*)>;

    template<class T>
    void assets_setup(const Assets*);

    class error : public std::runtime_error {
        AssetType type;
        std::string filename;
        std::string reason;
    public:
        error(
            AssetType type, std::string filename, std::string reason
        ) : std::runtime_error(filename + ": " + reason),
            type(type),
            filename(std::move(filename)),
            reason(std::move(reason)) {
        }

        AssetType getAssetType() const {
            return type;
        }

        const std::string& getFilename() const {
            return filename;
        }

        const std::string& getReason() const {
            return reason;
        }
    };
}

class Assets {
    std::vector<TextureAnimation> animations;

    using assets_map = std::unordered_map<std::string, std::shared_ptr<void>>;
    std::unordered_map<std::type_index, assets_map> assets;
    std::vector<assetload::setupfunc> setupFuncs;
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

    template<class T>
    std::optional<const assets_map*> getMap() const {
        const auto& mapIter = assets.find(typeid(T));
        if (mapIter == assets.end()) {
            return std::nullopt;
        }
        return &mapIter->second;
    }

    void setup() {
        for (auto& setupFunc : setupFuncs) {
            setupFunc(this);
        }
    }

    void addSetupFunc(assetload::setupfunc setupfunc) {
        setupFuncs.push_back(setupfunc);
    }
};

template<class T>
void assetload::assets_setup(const Assets* assets) {
    if (auto mapPtr = assets->getMap<T>()) {
        for (const auto& entry : **mapPtr) {
            static_cast<T*>(entry.second.get())->setup();
        }
    }
}

#endif // ASSETS_ASSETS_HPP_
