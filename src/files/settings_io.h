#ifndef FILES_SETTINGS_IO_H_
#define FILES_SETTINGS_IO_H_

#include <string>
#include "../settings.h"

namespace toml {
    class Wrapper;
}

extern std::string write_controls();
extern toml::Wrapper* create_wrapper(EngineSettings& settings);
extern void load_controls(std::string filename, std::string source);

#endif // FILES_SETTINGS_IO_H_