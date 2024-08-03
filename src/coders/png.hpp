#ifndef CODERS_PNG_HPP_
#define CODERS_PNG_HPP_

#include <memory>
#include <string>

class Texture;
class ImageData;

namespace png {
    std::unique_ptr<ImageData> load_image(const std::string& filename);
    void write_image(const std::string& filename, const ImageData* image);
    std::unique_ptr<Texture> load_texture(const std::string& filename);
}

#endif  // CODERS_PNG_HPP_
