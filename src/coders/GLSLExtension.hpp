#ifndef CODERS_GLSL_EXTESION_HPP_
#define CODERS_GLSL_EXTESION_HPP_

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>

class ResPaths;

class GLSLExtension {
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> defines;
    std::string version = "330 core";

    const ResPaths* paths = nullptr;
    void loadHeader(const std::string& name);
public:
    void setPaths(const ResPaths* paths);
    void setVersion(std::string version);

    void define(const std::string& name, std::string value);
    void undefine(const std::string& name);
    void addHeader(const std::string& name, std::string source);

    const std::string& getHeader(const std::string& name) const;
    const std::string getDefine(const std::string& name) const;

    bool hasHeader(const std::string& name) const;
    bool hasDefine(const std::string& name) const;

    const std::string process(
        const std::filesystem::path& file, 
        const std::string& source
    );
};

#endif // CODERS_GLSL_EXTESION_HPP_
