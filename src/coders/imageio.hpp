#pragma once

#include <memory>
#include <string>

#include "io/fwd.hpp"

class ImageData;

namespace imageio {
    inline const std::string PNG = ".png";

    bool is_read_supported(const std::string& extension);
    bool is_write_supported(const std::string& extension);

    std::unique_ptr<ImageData> read(const io::path& file);
    void write(const io::path& file, const ImageData* image);
}
