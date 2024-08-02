#include "files.hpp"

#include "../coders/commons.hpp"
#include "../coders/json.hpp"
#include "../coders/toml.hpp"
#include "../coders/gzip.hpp"
#include "../util/stringutil.hpp"
#include "../data/dynamic.hpp"

#include <fstream>
#include <iostream>
#include <memory>
#include <stdint.h>
#include <stdexcept>

namespace fs = std::filesystem;

files::rafile::rafile(const fs::path& filename)
    : file(filename, std::ios::binary | std::ios::ate) {
    if (!file) {
        throw std::runtime_error("could not to open file "+filename.string());
    }
    filelength = file.tellg();
    file.seekg(0);
}

size_t files::rafile::length() const {
    return filelength;
}

void files::rafile::seekg(std::streampos pos) {
    file.seekg(pos);
}

void files::rafile::read(char* buffer, std::streamsize size) {
    file.read(buffer, size);
}

bool files::write_bytes(const fs::path& filename, const ubyte* data, size_t size) {
    std::ofstream output(filename, std::ios::binary);
    if (!output.is_open())
        return false;
    output.write((const char*)data, size);
    output.close();
    return true;
}

uint files::append_bytes(const fs::path& filename, const ubyte* data, size_t size) {
    std::ofstream output(filename, std::ios::binary | std::ios::app);
    if (!output.is_open())
        return 0;
    uint position = output.tellp();
    output.write((const char*)data, size);
    output.close();
    return position;
}

bool files::read(const fs::path& filename, char* data, size_t size) {
    std::ifstream output(filename, std::ios::binary);
    if (!output.is_open())
        return false;
    output.read(data, size);
    output.close();
    return true;
}

std::unique_ptr<ubyte[]> files::read_bytes(const fs::path& filename, size_t& length) {
    std::ifstream input(filename, std::ios::binary);
    if (!input.is_open())
        return nullptr;
    input.seekg(0, std::ios_base::end);
    length = input.tellg();
    input.seekg(0, std::ios_base::beg);

    auto data = std::make_unique<ubyte[]>(length);
    input.read((char*)data.get(), length);
    input.close();
    return data;
}

std::vector<ubyte> files::read_bytes(const fs::path& filename) {
    std::ifstream input(filename, std::ios::binary);
    if (!input.is_open())
        return {};
    input.seekg(0, std::ios_base::end);
    size_t length = input.tellg();
    input.seekg(0, std::ios_base::beg);

    std::vector<ubyte> data(length);
    data.resize(length);
    input.read((char*)data.data(), length);
    input.close();
    return data;
}

std::string files::read_string(const fs::path& filename) {
    size_t size;
    std::unique_ptr<ubyte[]> bytes (read_bytes(filename, size));
    if (bytes == nullptr) {
        throw std::runtime_error("could not to load file '"+
                                 filename.string()+"'");
    }
    return std::string((const char*)bytes.get(), size);
}

bool files::write_string(const fs::path& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file) {
        return false;
    }
    file << content;
    return true;
}

bool files::write_json(const fs::path& filename, const dynamic::Map* obj, bool nice) {
    return files::write_string(filename, json::stringify(obj, nice, "  "));
}

bool files::write_binary_json(const fs::path& filename, const dynamic::Map* obj, bool compression) {
    auto bytes = json::to_binary(obj, compression);
    return files::write_bytes(filename, bytes.data(), bytes.size());
}

std::shared_ptr<dynamic::Map> files::read_json(const fs::path& filename) {
    std::string text = files::read_string(filename);
    return json::parse(filename.string(), text);
}

std::shared_ptr<dynamic::Map> files::read_binary_json(const fs::path& file) {
    size_t size;
    std::unique_ptr<ubyte[]> bytes (files::read_bytes(file, size));
    return json::from_binary(bytes.get(), size);
}

std::shared_ptr<dynamic::Map> files::read_toml(const fs::path& file) {
    return toml::parse(file.u8string(), files::read_string(file));
}

std::vector<std::string> files::read_list(const fs::path& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("could not to open file "+filename.u8string());
    }
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        util::trim(line);
        if (line.length() == 0)
            continue;
        if (line[0] == '#')
            continue;
        lines.push_back(line);
    }
    return lines;
}
