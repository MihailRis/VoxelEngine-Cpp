#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "io/io.hpp"
#include "graphics/core/PostEffect.hpp"

class ResPaths;

class GLSLExtension {
public:
    void setPaths(const ResPaths* paths);

    void define(const std::string& name, std::string value);
    void undefine(const std::string& name);
    void addHeader(const std::string& name, std::string source);

    const std::string& getHeader(const std::string& name) const;
    const std::string& getDefine(const std::string& name) const;

    const std::unordered_map<std::string, std::string>& getDefines() const;

    bool hasHeader(const std::string& name) const;
    bool hasDefine(const std::string& name) const;
    void loadHeader(const std::string& name);

    struct ProcessingResult {
        std::string code;
        std::unordered_map<std::string, PostEffect::Param> params;
    };

    ProcessingResult process(
        const io::path& file,
        const std::string& source,
        bool header = false
    );

    static inline std::string VERSION = "330 core";
private:
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> defines;

    const ResPaths* paths = nullptr;
};
