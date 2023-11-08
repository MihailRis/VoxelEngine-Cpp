#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include "../typedefs.h"

namespace files {
    extern bool write_bytes(std::string filename, const char* data, size_t size);
    extern uint append_bytes(std::string filename, const char* data, size_t size);
    extern bool read(std::string filename, char* data, size_t size);
    extern char* read_bytes(std::string filename, size_t& length);
    extern std::string read_string(std::string filename);
    extern bool write_string(std::string filename, const std::string content);
}

#endif /* FILES_FILES_H_ */
