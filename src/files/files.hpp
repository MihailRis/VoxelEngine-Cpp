#pragma once

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "typedefs.hpp"
#include "data/dv.hpp"
#include "util/Buffer.hpp"

namespace fs = std::filesystem;

namespace files {
    /// @brief Read-only random access file
    class rafile {
        std::ifstream file;
        size_t filelength;
    public:
        rafile(const fs::path& filename);

        void seekg(std::streampos pos);
        void read(char* buffer, std::streamsize size);
        size_t length() const;
    };

    /// @brief Write bytes array to the file without any extra data
    /// @param file target file
    /// @param data data bytes array
    /// @param size size of data bytes array
    bool write_bytes(const fs::path& file, const ubyte* data, size_t size);

    /// @brief Append bytes array to the file without any extra data
    /// @param file target file
    /// @param data data bytes array
    /// @param size size of data bytes array
    uint append_bytes(const fs::path& file, const ubyte* data, size_t size);

    /// @brief Write string to the file
    bool write_string(const fs::path& filename, std::string_view content);

    /// @brief Write dynamic data to the JSON file
    /// @param nice if true, human readable format will be used, otherwise
    /// minimal
    bool write_json(
        const fs::path& filename, const dv::value& obj, bool nice = true
    );

    /// @brief Write dynamic data to the binary JSON file
    /// (see src/coders/binary_json_spec.md)
    /// @param compressed use gzip compression
    bool write_binary_json(
        const fs::path& filename,
        const dv::value& obj,
        bool compressed = false
    );

    bool read(const fs::path&, char* data, size_t size);
    util::Buffer<ubyte> read_bytes_buffer(const fs::path&);
    std::unique_ptr<ubyte[]> read_bytes(const fs::path&, size_t& length);
    std::vector<ubyte> read_bytes(const fs::path&);
    std::string read_string(const fs::path& filename);

    /// @brief Read JSON or BJSON file
    /// @param file *.json or *.bjson file
    dv::value read_json(const fs::path& file);
    
    dv::value read_binary_json(const fs::path& file);
    
    /// @brief Read TOML file
    /// @param file *.toml file
    dv::value read_toml(const fs::path& file);

    std::vector<std::string> read_list(const fs::path& file);

    bool is_data_file(const fs::path& file);
    bool is_data_interchange_format(const fs::path& ext);
    dv::value read_object(const fs::path& file);
}
