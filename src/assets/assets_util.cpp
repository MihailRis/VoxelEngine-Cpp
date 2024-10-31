#include "assets_util.hpp"

#include "assets/Assets.hpp"
#include "graphics/core/Atlas.hpp"
#include "graphics/core/Texture.hpp"

util::TextureRegion util::getTextureRegion(
    const Assets& assets, const std::string& name, const std::string& fallback
) {
    size_t sep = name.find(':');
    if (sep == std::string::npos) {
        return {assets.get<Texture>(name), UVRegion(0,0,1,1)};
    } else {
        auto atlas = assets.get<Atlas>(name.substr(0, sep));
        if (atlas) {
            if (auto reg = atlas->getIf(name.substr(sep+1))) {
                return {atlas->getTexture(), *reg};
            } else if (!fallback.empty()){
                return util::getTextureRegion(assets, fallback, "");
            }
        }
    }
    return {nullptr, UVRegion()};
}
