#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <vector>
#include <filesystem>
#include "../typedefs.h"

namespace json {
    class JObject;
}

namespace files {
    extern bool write_bytes(std::filesystem::path, const char* data, size_t size);
    extern uint append_bytes(std::filesystem::path, const char* data, size_t size);
    extern bool read(std::filesystem::path, char* data, size_t size);
    extern char* read_bytes(std::filesystem::path, size_t& length);
    extern std::string read_string(std::filesystem::path filename);
    extern bool write_string(std::filesystem::path filename, const std::string content);
    extern json::JObject* read_json(std::filesystem::path file);
    extern std::vector<std::string> read_list(std::filesystem::path file);
}

#endif /* FILES_FILES_H_ */