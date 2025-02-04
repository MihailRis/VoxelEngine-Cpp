#include "path.hpp"

#include <stack>

using namespace io;

void path::checkValid() const {
    if (colonPos == std::string::npos) {
        throw std::runtime_error("path entry point is not specified: " + str);
    }
}

path path::normalized() const {
    io::path path = pathPart();

    std::stack<std::string> parts;
    do {
        parts.push(path.name());
        path.str = path.parent().string();
    } while (!path.empty());

    while (!parts.empty()) {
        const std::string part = parts.top();
        parts.pop();
        if (part == ".") {
            continue;
        }
        if (part == "..") {
            throw access_error("entry point reached");
        }

        path = path / part;
    }
    if (path.colonPos != std::string::npos) {
        path = path.entryPoint() + ":" + path.string();
    }
    return path;
}
