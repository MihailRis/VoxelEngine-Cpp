#ifndef FILES_FILES_HPP_
#define FILES_FILES_HPP_

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <typedefs.hpp>

namespace fs = std::filesystem;

namespace dynamic {
    class Map;
}

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
    bool write_string(const fs::path& filename, const std::string content);

    /// @brief Write dynamic data to the JSON file
    /// @param nice if true, human readable format will be used, otherwise
    /// minimal
    bool write_json(
        const fs::path& filename, const dynamic::Map* obj, bool nice = true
    );

    /// @brief Write dynamic data to the binary JSON file
    /// (see src/coders/binary_json_spec.md)
    /// @param compressed use gzip compression
    bool write_binary_json(
        const fs::path& filename,
        const dynamic::Map* obj,
        bool compressed = false
    );

    bool read(const fs::path&, char* data, size_t size);
    std::unique_ptr<ubyte[]> read_bytes(const fs::path&, size_t& length);
    std::vector<ubyte> read_bytes(const fs::path&);
    std::string read_string(const fs::path& filename);

    /// @brief Read JSON or BJSON file
    /// @param file *.json or *.bjson file
    std::shared_ptr<dynamic::Map> read_json(const fs::path& file);
    std::shared_ptr<dynamic::Map> read_binary_json(const fs::path& file);
    std::shared_ptr<dynamic::Map> read_toml(const fs::path& file);
    std::vector<std::string> read_list(const fs::path& file);
}

#endif /* FILES_FILES_HPP_ */
