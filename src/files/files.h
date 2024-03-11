#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <filesystem>
#include "../typedefs.h"

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
        rafile(std::filesystem::path filename);

        void seekg(std::streampos pos);
        void read(char* buffer, std::streamsize size);
        size_t length() const;
    };

    /// @brief Write bytes array to the file without any extra data
    /// @param file target file
    /// @param data data bytes array
    /// @param size size of data bytes array
    extern bool write_bytes(fs::path file, const ubyte* data, size_t size);

    /// @brief Append bytes array to the file without any extra data
    /// @param file target file
    /// @param data data bytes array
    /// @param size size of data bytes array
    extern uint append_bytes(fs::path file, const ubyte* data, size_t size);

    /// @brief Write string to the file
    extern bool write_string(fs::path filename, const std::string content);

    /// @brief Write dynamic data to the JSON file
    /// @param nice if true, human readable format will be used, otherwise minimal
    extern bool write_json(
        fs::path filename, 
        const dynamic::Map* obj, 
        bool nice=true);

    /// @brief Write dynamic data to the binary JSON file
    /// (see src/coders/binary_json_spec.md)
    /// @param compressed use gzip compression
    extern bool write_binary_json(
        fs::path filename, 
        const dynamic::Map* obj, 
        bool compressed=false
    );

    extern bool read(fs::path, char* data, size_t size);
    extern ubyte* read_bytes(fs::path, size_t& length);
    extern std::string read_string(fs::path filename);

    /// @brief Read JSON or BJSON file
    /// @param file *.json or *.bjson file
    extern std::unique_ptr<dynamic::Map> read_json(fs::path file);
    extern std::unique_ptr<dynamic::Map> read_binary_json(fs::path file);
    extern std::vector<std::string> read_list(fs::path file);
}

#endif /* FILES_FILES_H_ */