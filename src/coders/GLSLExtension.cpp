#include "GLSLExtension.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "../util/stringutil.h"
#include "../typedefs.h"
#include "../files/files.h"
#include "../files/engine_paths.h"

using std::string;
using std::filesystem::path;
namespace fs = std::filesystem;

void GLSLExtension::setVersion(string version) {
    this->version = version;
}

void GLSLExtension::setPaths(const ResPaths* paths) {
    this->paths = paths;
}

void GLSLExtension::loadHeader(string name) {
    path file = paths->find("shaders/lib/"+name+".glsl");
    string source = files::read_string(file);
    addHeader(name, source);
}

void GLSLExtension::addHeader(string name, string source) {
    headers[name] = source;
}

void GLSLExtension::define(string name, string value) {
    defines[name] = value;
}

const string& GLSLExtension::getHeader(const string name) const {
    auto found = headers.find(name);
    if (found == headers.end()) {
        throw std::runtime_error("no header '"+name+"' loaded");
    }
    return found->second;
}

const string GLSLExtension::getDefine(const string name) const {
    auto found = defines.find(name);
    if (found == defines.end()) {
        return "";
    }
    return found->second;
}

bool GLSLExtension::hasDefine(const string name) const {
    return defines.find(name) != defines.end();
}

bool GLSLExtension::hasHeader(const string name) const {
    return headers.find(name) != headers.end();
}

void GLSLExtension::undefine(string name) {
    if (hasDefine(name)) {
        defines.erase(name);
    }
}

inline std::runtime_error parsing_error(
        const path& file, 
        uint linenum, 
        const string message) {
    return std::runtime_error("file "+file.string()+": "+message+
                          " at line "+std::to_string(linenum));
}

inline void parsing_warning(
        const path& file, 
        uint linenum, const 
        string message) {
    std::cerr << "file "+file.string()+": warning: "+message+
             " at line "+std::to_string(linenum) << std::endl;
}

inline void source_line(std::stringstream& ss, uint linenum) {
    ss << "#line " << linenum << "\n";
}

const string GLSLExtension::process(const path file, const string& source) {
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
        if (endline == string::npos) {
            endline = source.length();
        }
        // parsing preprocessor directives
        if (source[pos] == '#') {
            string line = source.substr(pos+1, endline-pos);
            util::trim(line);
            // parsing 'include' directive
            if (line.find("include") != string::npos) {
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
                string name = line.substr(1, line.length()-2);
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
            else if (line.find("version") != string::npos) {
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