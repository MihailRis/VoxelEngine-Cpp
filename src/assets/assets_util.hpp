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

    TextureRegion get_texture_region(
        const Assets& assets,
        const std::string& name,
        const std::string& fallback
    );
}
