#include "imageio.hpp"

#include <functional>
#include <unordered_map>

#include "graphics/core/ImageData.hpp"
#include "io/io.hpp"
#include "png.hpp"

using image_reader =
    std::function<std::unique_ptr<ImageData>(const ubyte*, size_t)>;
using image_writer = std::function<void(const std::string&, const ImageData*)>;

static std::unordered_map<std::string, image_reader> readers {
    {".png", png::load_image},
};

static std::unordered_map<std::string, image_writer> writers {
    {".png", png::write_image},
};

bool imageio::is_read_supported(const std::string& extension) {
    return readers.find(extension) != readers.end();
}

bool imageio::is_write_supported(const std::string& extension) {
    return writers.find(extension) != writers.end();
}

std::unique_ptr<ImageData> imageio::read(const io::path& file) {
    auto found = readers.find(file.extension());
    if (found == readers.end()) {
        throw std::runtime_error(
            "file format is not supported (read): " + file.string()
        );
    }
    auto bytes = io::read_bytes_buffer(file);
    try {
        return std::unique_ptr<ImageData>(found->second(bytes.data(), bytes.size()));
    } catch (const std::runtime_error& err) {
        throw std::runtime_error(
            "could not to load image " + file.string() + ": " + err.what()
        );
    }
}

void imageio::write(const io::path& file, const ImageData* image) {
    auto found = writers.find(file.extension());
    if (found == writers.end()) {
        throw std::runtime_error(
            "file format is not supported (write): " + file.string()
        );
    }
    return found->second(io::resolve(file).u8string(), image);
}
