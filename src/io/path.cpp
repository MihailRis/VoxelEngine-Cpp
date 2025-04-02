#include "path.hpp"

#include <stack>

using namespace io;

void path::checkValid() const {
    if (colonPos == std::string::npos) {
        throw std::runtime_error("path entry point is not specified: " + str);
    }
}

path path::parent() const {
    size_t length = str.length();
    while (length && str[length-1] == '/') {
        length--;
    }
    size_t slashpos = length;
    slashpos = str.rfind('/', slashpos-1);
    if (length >= 2 && str.rfind("..") == length - 2) {
        return normalized().parent();
    }
    if (slashpos == std::string::npos) {
        return colonPos == std::string::npos ? "" : str.substr(0, colonPos+1);
    }
    while (slashpos && str[slashpos-1] == '/') {
        slashpos--;
    }
    return str.substr(0, slashpos);
}

path path::normalized() const {
    io::path path = pathPart();

    std::stack<io::path> parts;
    int64_t pos = 0;
    int64_t prev = pos-1;
    while (pos < path.str.length()) {
        pos = path.str.find('/', pos);
        if (pos == std::string::npos) {
            parts.push(path.str.substr(prev + 1));
            break;
        }
        if (pos - prev == 0) {
            prev = pos;
            pos = prev + 1;
            continue;
        }
        auto token = path.str.substr(prev + 1, pos - (prev + 1));
        prev = pos;
        if (token == ".") {
            continue;
        } else if (token == "..") {
            if (parts.empty()) {
                throw access_error("entry-point reached");
            }
            parts.pop();
            continue;
        }
        parts.push(std::move(token));
    }
    path = "";
    while (!parts.empty()) {
        const auto& token = parts.top();
        if (path.empty()) {
            path = token;
        } else {
            path = token / path;
        }
        parts.pop();
    }
    if (colonPos != std::string::npos) {
        path = str.substr(0, colonPos+1) + path.string();
    }
    return path;
}
