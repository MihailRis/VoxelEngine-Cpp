#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include <filesystem>
#include "../typedefs.h"

namespace json {
    class JObject;
}

namespace files {
    extern bool write_bytes(const std::filesystem::path&, const char* data, size_t size);
    extern uint append_bytes(const std::filesystem::path&, const char* data, size_t size);
    extern bool read(const std::filesystem::path&, char* data, size_t size);
    extern char* read_bytes(const std::filesystem::path&, size_t& length);
    extern std::string read_string(const std::filesystem::path& filename);
    extern bool write_string(const std::filesystem::path& filename, const std::string& content);
    extern json::JObject* read_json(const std::filesystem::path& file);
}

#endif /* FILES_FILES_H_ */