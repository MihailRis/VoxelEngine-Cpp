#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

#include "ContentPack.hpp"

class Content;
struct ContentPack;
class EnginePaths;
class Input;

namespace io {
    class path;
}

class ContentControl {
public:
    ContentControl(
        EnginePaths& paths, Input& input, std::function<void()> postContent
    );
    ~ContentControl();

    Content* get();

    const Content* get() const;

    std::vector<std::string>& getBasePacks();

    /// @brief Reset content to base packs list
    void resetContent();

    void loadContent(const std::vector<std::string>& names);

    void loadContent();

    std::vector<io::path> getDefaultSources();
    std::vector<ContentPack>& getContentPacks();
    std::vector<ContentPack> getAllContentPacks();
private:
    EnginePaths& paths;
    Input& input;
    std::unique_ptr<Content> content;
    std::function<void()> postContent;
    std::vector<std::string> basePacks;
    std::vector<ContentPack> contentPacks;
};
