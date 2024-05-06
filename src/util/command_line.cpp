#include "command_line.hpp"

#include <filesystem>

namespace fs = std::filesystem;

bool perform_keyword(ArgsReader& reader, const std::string& keyword, EnginePaths& paths) {
    if (keyword == "--res") {
        auto token = reader.next();
        if (!fs::is_directory(fs::path(token))) {
            throw std::runtime_error(token+" is not a directory");
        }
        paths.setResources(fs::path(token));
        std::cout << "resources folder: " << token << std::endl;
    } else if (keyword == "--dir") {
        auto token = reader.next();
        if (!fs::is_directory(fs::path(token))) {
            fs::create_directories(fs::path(token));
        }
        paths.setUserfiles(fs::path(token));
        std::cout << "userfiles folder: " << token << std::endl;
    } else if (keyword == "--help" || keyword == "-h") {
        std::cout << "VoxelEngine command-line arguments:" << std::endl;
        std::cout << " --res [path] - set resources directory" << std::endl;
        std::cout << " --dir [path] - set userfiles directory" << std::endl;
        return false;
    } else {
        std::cerr << "unknown argument " << keyword << std::endl;
        return false;
    }
    return true;
}

bool parse_cmdline(int argc, char** argv, EnginePaths& paths) {
    ArgsReader reader(argc, argv);
    reader.skip();
    while (reader.hasNext()) {
        std::string token = reader.next();
        if (reader.isKeywordArg()) {
            if (!perform_keyword(reader, token, paths)) {
                return false;
            }
        } else {
            std::cerr << "unexpected token" << std::endl;
            return false;
        }
    }
    return true;
}
