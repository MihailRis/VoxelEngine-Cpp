#ifndef CODERS_GLSL_EXTESION_H_
#define CODERS_GLSL_EXTESION_H_

#include <string>
#include <unordered_map>
#include <filesystem>

class GLSLExtension {
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> defines;
    std::filesystem::path libFolder;
    std::string version = "330 core";

    void loadHeader(const std::string& name);
    std::filesystem::path getHeaderPath(const std::string& name);
public:
    void setLibFolder(const std::filesystem::path& folder);
    void setVersion(const std::string_view& version);

    void define(const std::string& name, const std::string& value);
    void undefine(const std::string& name);
    void addHeader(const std::string& name, const std::string& source);

    const std::string& getHeader(const std::string& name) const;
    const std::string getDefine(const std::string& name) const;

    bool hasHeader(const std::string& name) const;
    bool hasDefine(const std::string& name) const;

    const std::string process(const std::filesystem::path& file, const std::string& source);
};

#endif // CODERS_GLSL_EXTESION_H_