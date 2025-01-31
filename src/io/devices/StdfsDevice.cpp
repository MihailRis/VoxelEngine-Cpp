#include "StdfsDevice.hpp"

#include <fstream>
#include <filesystem>

using namespace io;
namespace fs = std::filesystem;

fs::path StdfsDevice::resolve(std::string_view path) {
    return root / fs::u8path(path);
}

void StdfsDevice::write(std::string_view path, const void* data, size_t size) {
    auto resolved = resolve(path);
    std::ofstream output(resolved, std::ios::binary);
    if (!output.is_open()) {
        throw std::runtime_error("could not to open file " + resolved.u8string());
    }
    output.write((const char*)data, size);
}

void StdfsDevice::read(std::string_view path, void* data, size_t size) {
    auto resolved = resolve(path);
    std::ifstream input(resolved, std::ios::binary);
    if (!input.is_open()) {
        throw std::runtime_error("could not to open file " + resolved.u8string());
    }
    input.read((char*)data, size);
}

size_t StdfsDevice::size(std::string_view path) {
    auto resolved = resolve(path);
    return fs::file_size(resolved);
}

bool StdfsDevice::exists(std::string_view path) {
    auto resolved = resolve(path);
    return fs::exists(resolved);
}

bool StdfsDevice::isdir(std::string_view path) {
    auto resolved = resolve(path);
    return fs::is_directory(resolved);
}

bool StdfsDevice::isfile(std::string_view path) {
    auto resolved = resolve(path);
    return fs::is_regular_file(resolved);
}

void StdfsDevice::mkdirs(std::string_view path) {
    auto resolved = resolve(path);
    fs::create_directories(resolved);
}

bool StdfsDevice::remove(std::string_view path) {
    auto resolved = resolve(path);
    return fs::remove(resolved);
}

uint64_t StdfsDevice::removeAll(std::string_view path) {
    auto resolved = resolve(path);
    return fs::remove_all(resolved);
}

class StdfsPathsGenerator : public PathsGenerator {
public:
    StdfsPathsGenerator(fs::path root) : root(std::move(root)) {
        it = fs::directory_iterator(this->root);
    }

    bool next(io::path& path) override {
        if (it == fs::directory_iterator()) {
            return false;
        }
        path = it->path().filename().u8string();
        it++;
        return true;
    }
private:
    fs::path root;
    fs::directory_iterator it;
};

std::unique_ptr<PathsGenerator> StdfsDevice::list(std::string_view path) {
    return std::make_unique<StdfsPathsGenerator>(root / fs::u8path(path));
}
