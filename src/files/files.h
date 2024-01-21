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
    /* Read-only random access file */
    class rafile {
        std::ifstream file;
        size_t filelength;
    public:
        rafile(std::filesystem::path filename);

        void seekg(std::streampos pos);
        void read(char* buffer, std::streamsize size);
        size_t length() const;
    };

    /* Write bytes array to the file without any extra data */
    extern bool write_bytes(fs::path, const ubyte* data, size_t size);

    /* Append bytes array to the file without any extra data */
    extern uint append_bytes(fs::path, const ubyte* data, size_t size);

    /* Write string to the file */
    extern bool write_string(fs::path filename, const std::string content);

    /* Write dynamic data to the JSON file
       @param nice if true, 
         human readable format will be used, otherwise minimal */
    extern bool write_json(
        fs::path filename, 
        const dynamic::Map* obj, 
        bool nice=true);

    /* Write dynamic data to the binary JSON file
       (see src/coders/binary_json_spec.md)
       @param compressed use gzip compression */
    extern bool write_binary_json(
        fs::path filename, 
        const dynamic::Map* obj, 
        bool compressed=false);

    extern bool read(fs::path, char* data, size_t size);
    extern ubyte* read_bytes(fs::path, size_t& length);
    extern std::string read_string(fs::path filename);
    extern std::unique_ptr<dynamic::Map> read_json(fs::path file);
    extern std::unique_ptr<dynamic::Map> read_binary_json(fs::path file);
    extern std::vector<std::string> read_list(fs::path file);
}

#endif /* FILES_FILES_H_ */