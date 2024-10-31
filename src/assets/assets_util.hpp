#pragma once

#include <string>

#include "maths/UVRegion.hpp"

class Assets;
class Texture;

namespace util {
    struct TextureRegion {
        const Texture* texture;
        UVRegion region;
    };

    TextureRegion getTextureRegion(
        const Assets& assets,
        const std::string& name,
        const std::string& fallback
    );
}
