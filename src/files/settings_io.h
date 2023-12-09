#ifndef FILES_SETTINGS_IO_H_
#define FILES_SETTINGS_IO_H_

#include <string>
#include "../coders/toml.h"
#include "../settings.h"

extern std::string write_controls();
extern toml::Wrapper create_wrapper(EngineSettings& settings);
extern void load_controls(const std::string& filename, const std::string& source);

#endif // FILES_SETTINGS_IO_H_