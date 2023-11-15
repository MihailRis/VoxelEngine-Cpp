#ifndef FILES_ENGINE_FILES_H_
#define FILES_ENGINE_FILES_H_

#include <string>
#include <filesystem>

#define SCREENSHOTS_FOLDER "screenshots"

namespace enginefs {
    extern std::filesystem::path get_screenshot_file(std::string ext);
    extern std::filesystem::path get_worlds_folder();
}

#endif // FILES_ENGINE_FILES_H_