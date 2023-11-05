#ifndef FILES_FILES_H_
#define FILES_FILES_H_

#include <string>
#include "../typedefs.h"

extern bool write_binary_file(std::string filename, const char* data, size_t size);
extern unsigned int append_binary_file(std::string filename, const char* data, size_t size);
extern bool read_binary_file(std::string filename, char* data, size_t size);
extern char* read_binary_file(std::string filename, size_t& length);

#endif /* FILES_FILES_H_ */
