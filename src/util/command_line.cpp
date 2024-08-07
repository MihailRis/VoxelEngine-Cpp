#include "command_line.hpp"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include <files/engine_paths.hpp>

namespace fs = std::filesystem;

class ArgsReader {
    const char* last = "";
    char** argv;
    int argc;
    int pos = 0;
public:
    ArgsReader(int argc, char** argv) : argv(argv), argc(argc) {
    }

    void skip() {
        pos++;
    }

    bool hasNext() const {
        return pos < argc && strlen(argv[pos]);
    }

    bool isKeywordArg() const {
        return last[0] == '-';
    }

    std::string next() {
        if (pos >= argc) {
            throw std::runtime_error("unexpected end");
        }
        last = argv[pos];
        return argv[pos++];
    }
};

bool perform_keyword(
    ArgsReader& reader, const std::string& keyword, EnginePaths& paths
) {
    if (keyword == "--res") {
        auto token = reader.next();
        if (!fs::is_directory(fs::path(token))) {
            throw std::runtime_error(token + " is not a directory");
        }
        paths.setResourcesFolder(fs::path(token));
        std::cout << "resources folder: " << token << std::endl;
    } else if (keyword == "--dir") {
        auto token = reader.next();
        if (!fs::is_directory(fs::path(token))) {
            fs::create_directories(fs::path(token));
        }
        paths.setUserFilesFolder(fs::path(token));
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
