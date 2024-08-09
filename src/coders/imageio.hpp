#pragma once

#include <memory>
#include <string>

class ImageData;

namespace imageio {
    inline const std::string PNG = ".png";

    bool is_read_supported(const std::string& extension);
    bool is_write_supported(const std::string& extension);

    std::unique_ptr<ImageData> read(const std::string& filename);
    void write(const std::string& filename, const ImageData* image);
}
