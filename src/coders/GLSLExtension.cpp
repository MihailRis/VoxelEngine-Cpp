#include "GLSLExtension.hpp"

#include <sstream>
#include <stdexcept>
#include <utility>

#include "debug/Logger.hpp"
#include "io/engine_paths.hpp"
#include "typedefs.hpp"
#include "util/stringutil.hpp"
#include "coders/BasicParser.hpp"
#include "graphics/core/PostEffect.hpp"

static debug::Logger logger("glsl-extension");

using Type = PostEffect::Param::Type;

void GLSLExtension::setPaths(const ResPaths* paths) {
    this->paths = paths;
}

void GLSLExtension::loadHeader(const std::string& name) {
    if (paths == nullptr) {
        return;
    }
    io::path file = paths->find("shaders/lib/" + name + ".glsl");
    std::string source = io::read_string(file);
    addHeader(name, "");
    addHeader(name, process(file, source, true));
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
        throw std::runtime_error("no header '" + name + "' loaded");
    }
    return found->second;
}

const std::string& GLSLExtension::getDefine(const std::string& name) const {
    auto found = defines.find(name);
    if (found == defines.end()) {
        throw std::runtime_error("name '" + name + "' is not defined");
    }
    return found->second;
}

const std::unordered_map<std::string, std::string>& GLSLExtension::getDefines() const {
    return defines;
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
    const io::path& file, uint linenum, const std::string& message
) {
    return std::runtime_error(
        "file " + file.string() + ": " + message + " at line " +
        std::to_string(linenum)
    );
}

inline void parsing_warning(
    std::string_view file, uint linenum, const std::string& message
) {
    logger.warning() << "file " + std::string(file) + ": warning: " + message +
                     " at line " + std::to_string(linenum);
}

inline void source_line(std::stringstream& ss, uint linenum) {
    ss << "#line " << linenum << "\n";
}

static std::optional<PostEffect::Param::Type> param_type_from(
    const std::string& name
) {
    static const std::unordered_map<std::string, PostEffect::Param::Type> typeNames {
        {"float", Type::FLOAT},
        {"vec2", Type::VEC2},
        {"vec3", Type::VEC3},
        {"vec4", Type::VEC4},
    };
    const auto& found = typeNames.find(name);
    if (found == typeNames.end()) {
        return std::nullopt;
    }
    return found->second;
}

class GLSLParser : public BasicParser<char> {
public:
    GLSLParser(GLSLExtension& glsl, std::string_view file, std::string_view source, bool header)
        : BasicParser(file, source), glsl(glsl) {
        if (!header) {
            ss << "#version " << GLSLExtension::VERSION << '\n';
        }
        for (auto& entry : glsl.getDefines()) {
            ss << "#define " << entry.first << " " << entry.second << '\n';
        }
        uint linenum = 1;
        source_line(ss, linenum);

        clikeComment = true;
    }

    bool processPreprocessorDirective() {
        skip(1);

        auto name = parseName();

        if (name == "version") {
            parsing_warning(filename, line, "removed #version directive");
            source_line(ss, line);
            skipLine();
            return false;
        } else if (name == "include") {
            skipWhitespace(false);
            if (peekNoJump() != '<') {
                throw error("'<' expected");
            }
            skip(1);
            skipWhitespace(false);
            auto headerName = parseName();
            skipWhitespace(false);
            if (peekNoJump() != '>') {
                throw error("'>' expected");
            }
            skip(1);
            skipWhitespace(false);
            skipLine();

            if (!glsl.hasHeader(headerName)) {
                glsl.loadHeader(headerName);
            }
            ss << glsl.getHeader(headerName) << '\n';
            source_line(ss, line);
            return false;
        } else if (name == "param") {
            skipWhitespace(false);
            auto typeName = parseName();
            auto type = param_type_from(typeName);
            if (!type.has_value()) {
                throw error("unsupported param type " + util::quote(typeName));
            }
            skipWhitespace(false);
            auto paramName = parseName();
            if (params.find(paramName) != params.end()) {
                throw error("duplicating param " + util::quote(paramName));
            }
            skipLine();

            ss << "uniform " << typeName << " " << paramName << ";\n";
            params[paramName] = PostEffect::Param(type.value());
            return false;
        }
        return true;
    }

    std::string process() {
        while (hasNext()) {
            skipWhitespace(false);
            if (!hasNext()) {
                break;
            }
            if (source[pos] != '#' || processPreprocessorDirective()) {
                pos = linestart;
                ss << readUntilEOL() << '\n';
                skip(1);
            }
        }
        return ss.str();
    }
private:
    GLSLExtension& glsl;
    std::unordered_map<std::string, PostEffect::Param> params;
    std::stringstream ss;
};

std::string GLSLExtension::process(
    const io::path& file, const std::string& source, bool header
) {
    std::string filename = file.string();
    GLSLParser parser(*this, filename, source, header);
    return parser.process();
}
