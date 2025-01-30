#include "StdfsDevice.hpp"

#include <fstream>
#include <filesystem>

using namespace io;

std::filesystem::path StdfsDevice::resolve(std::string_view path) {
    return root / std::filesystem::u8path(path);
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
    return std::filesystem::file_size(resolved);
}

bool StdfsDevice::exists(std::string_view path) {
    auto resolved = resolve(path);
    return std::filesystem::exists(resolved);
}

bool StdfsDevice::isdir(std::string_view path) {
    auto resolved = resolve(path);
    return std::filesystem::is_directory(resolved);
}

bool StdfsDevice::isfile(std::string_view path) {
    auto resolved = resolve(path);
    return std::filesystem::is_regular_file(resolved);
}

void StdfsDevice::mkdirs(std::string_view path) {
    auto resolved = resolve(path);
    std::filesystem::create_directories(resolved);
}

bool StdfsDevice::remove(std::string_view path) {
    auto resolved = resolve(path);
    return std::filesystem::remove(resolved);
}

uint64_t StdfsDevice::removeAll(std::string_view path) {
    auto resolved = resolve(path);
    return std::filesystem::remove_all(resolved);
}
