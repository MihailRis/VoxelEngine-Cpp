#ifndef CODERS_IMAGEIO_HPP_
#define CODERS_IMAGEIO_HPP_

#include <string>
#include <memory>

class ImageData;

namespace imageio {
    inline const std::string PNG = ".png";

    bool is_read_supported(const std::string& extension);
    bool is_write_supported(const std::string& extension);

    std::unique_ptr<ImageData> read(const std::string& filename);
    void write(const std::string& filename, const ImageData* image);
}

#endif // CODERS_IMAGEIO_HPP_
