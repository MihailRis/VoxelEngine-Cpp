#include "command_line.hpp"

#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

#include "files/engine_paths.hpp"
#include "engine.hpp"

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

static bool perform_keyword(
    ArgsReader& reader, const std::string& keyword, CoreParameters& params
) {
    if (keyword == "--res") {
        auto token = reader.next();
        params.resFolder = fs::u8path(token);
    } else if (keyword == "--dir") {
        auto token = reader.next();
        params.userFolder = fs::u8path(token);
    } else if (keyword == "--help" || keyword == "-h") {
        std::cout << "VoxelEngine command-line arguments:\n";
        std::cout << " --help - show help\n";
        std::cout << " --res [path] - set resources directory\n";
        std::cout << " --dir [path] - set userfiles directory\n";
        std::cout << " --headless - run in headless mode\n";
        std::cout << std::endl;
        return false;
    } else if (keyword == "--headless") {
        params.headless = true;
    } else {
        std::cerr << "unknown argument " << keyword << std::endl;
        return false;
    }
    return true;
}

bool parse_cmdline(int argc, char** argv, CoreParameters& params) {
    ArgsReader reader(argc, argv);
    reader.skip();
    while (reader.hasNext()) {
        std::string token = reader.next();
        if (reader.isKeywordArg()) {
            if (!perform_keyword(reader, token, params)) {
                return false;
            }
        } else {
            std::cerr << "unexpected token" << std::endl;
            return false;
        }
    }
    return true;
}
