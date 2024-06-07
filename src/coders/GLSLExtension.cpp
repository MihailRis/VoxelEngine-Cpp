#include "GLSLExtension.hpp"

#include "../util/stringutil.hpp"
#include "../typedefs.hpp"
#include "../files/files.hpp"
#include "../files/engine_paths.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace fs = std::filesystem;

void GLSLExtension::setVersion(std::string version) {
    this->version = std::move(version);
}

void GLSLExtension::setPaths(const ResPaths* paths) {
    this->paths = paths;
}

void GLSLExtension::loadHeader(const std::string& name) {
    fs::path file = paths->find("shaders/lib/"+name+".glsl");
    std::string source = files::read_string(file);
    addHeader(name, source);
}

void GLSLExtension::addHeader(const std::string& name, std::string source) {
    headers[name] = std::move(source);
}

void GLSLExtension::define(const std::string& name, std::string value) {
    defines[name] = std::move(value);
}

const std::string& GLSLExtension::getHeader(const std::string& name) const {
    auto found = headers.find(name);
    if (found == headers.end()) {
        throw std::runtime_error("no header '"+name+"' loaded");
    }
    return found->second;
}

const std::string& GLSLExtension::getDefine(const std::string& name) const {
    auto found = defines.find(name);
    if (found == defines.end()) {
        throw std::runtime_error("name '"+name+"' is not defined");
    }
    return found->second;
}

bool GLSLExtension::hasDefine(const std::string& name) const {
    return defines.find(name) != defines.end();
}

bool GLSLExtension::hasHeader(const std::string& name) const {
    return headers.find(name) != headers.end();
}

void GLSLExtension::undefine(const std::string& name) {
    if (hasDefine(name)) {
        defines.erase(name);
    }
}

inline std::runtime_error parsing_error(
        const fs::path& file, 
        uint linenum, 
        const std::string& message) {
    return std::runtime_error("file "+file.string()+": "+message+
                          " at line "+std::to_string(linenum));
}

inline void parsing_warning(
        const fs::path& file, 
        uint linenum, const 
        std::string& message) {
    std::cerr << "file "+file.string()+": warning: "+message+
            " at line "+std::to_string(linenum) << std::endl;
}

inline void source_line(std::stringstream& ss, uint linenum) {
    ss << "#line " << linenum << "\n";
}

std::string GLSLExtension::process(const fs::path& file, const std::string& source) {
    std::stringstream ss;
    size_t pos = 0;
    uint linenum = 1;
    ss << "#version " << version << '\n';
    for (auto& entry : defines) {
        ss << "#define " << entry.first << " " << entry.second << '\n';
    }
    source_line(ss, linenum);
    while (pos < source.length()) {
        size_t endline = source.find('\n', pos);
        if (endline == std::string::npos) {
            endline = source.length();
        }
        // parsing preprocessor directives
        if (source[pos] == '#') {
            std::string line = source.substr(pos+1, endline-pos);
            util::trim(line);
            // parsing 'include' directive
            if (line.find("include") != std::string::npos) {
                line = line.substr(7);
                util::trim(line);
                if (line.length() < 3) {
                    throw parsing_error(file, linenum, 
                        "invalid 'include' syntax");
                }
                if (line[0] != '<' || line[line.length()-1] != '>') {
                    throw parsing_error(file, linenum,
                        "expected '#include <filename>' syntax");
                }
                std::string name = line.substr(1, line.length()-2);
                if (!hasHeader(name)) {
                    loadHeader(name);
                }
                source_line(ss, 1);
                ss << getHeader(name) << '\n';
                pos = endline+1;
                linenum++;
                source_line(ss, linenum);
                continue;
            } 
            // removing extra 'include' directives
            else if (line.find("version") != std::string::npos) {
                parsing_warning(file, linenum, "removed #version directive");
                pos = endline+1;
                linenum++;
                source_line(ss, linenum);
                continue;
            }
        }
        linenum++;
        ss << source.substr(pos, endline+1-pos);
        pos = endline+1;
    }
    return ss.str();    
}
