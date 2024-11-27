#include "imageio.hpp"

#include <filesystem>
#include <functional>
#include <unordered_map>

#include "graphics/core/ImageData.hpp"
#include "files/files.hpp"
#include "png.hpp"

namespace fs = std::filesystem;

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

inline std::string extensionOf(const std::string& filename) {
    return fs::u8path(filename).extension().u8string();
}

std::unique_ptr<ImageData> imageio::read(const fs::path& filename) {
    auto found = readers.find(extensionOf(filename.u8string()));
    if (found == readers.end()) {
        throw std::runtime_error(
            "file format is not supported (read): " + filename.u8string()
        );
    }
    auto bytes = files::read_bytes_buffer(filename);
    try {
        return std::unique_ptr<ImageData>(found->second(bytes.data(), bytes.size()));
    } catch (const std::runtime_error& err) {
        throw std::runtime_error(
            "could not to load image " + filename.u8string() + ": " + err.what()
        );
    }
}

void imageio::write(const std::string& filename, const ImageData* image) {
    auto found = writers.find(extensionOf(filename));
    if (found == writers.end()) {
        throw std::runtime_error(
            "file format is not supported (write): " + filename
        );
    }
    return found->second(filename, image);
}
