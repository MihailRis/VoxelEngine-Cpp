#pragma once

#include <memory>
#include <string>

#include "typedefs.hpp"

class Texture;
class ImageData;

namespace png {
    std::unique_ptr<ImageData> load_image_inmemory(const ubyte* bytes, size_t size);
    std::unique_ptr<ImageData> load_image(const std::string& filename);
    void write_image(const std::string& filename, const ImageData* image);
    std::unique_ptr<Texture> load_texture(const std::string& filename);
}
