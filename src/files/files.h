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


    extern bool write_bytes(fs::path, const char* data, size_t size);
    extern uint append_bytes(fs::path, const char* data, size_t size);
    extern bool write_string(fs::path filename, const std::string content);
    extern bool write_json(fs::path filename, const dynamic::Map* obj, bool nice=true);
    extern bool write_binary_json(fs::path filename, const dynamic::Map* obj);

    extern bool read(fs::path, char* data, size_t size);
    extern char* read_bytes(fs::path, size_t& length);
    extern std::string read_string(fs::path filename);
    extern std::unique_ptr<dynamic::Map> read_json(fs::path file);
    extern std::unique_ptr<dynamic::Map> read_binary_json(fs::path file);
    extern std::vector<std::string> read_list(fs::path file);
}

#endif /* FILES_FILES_H_ */