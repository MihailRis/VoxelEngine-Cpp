#pragma once

#include <memory>
#include <vector>
#include <string>
#include <functional>

class Content;
struct ContentPack;
class EnginePaths;
class Input;

namespace io {
    class path;
}

class ContentControl {
public:
    ContentControl(std::function<void()> postContent);
    ~ContentControl();

    Content* get();

    std::vector<std::string>& getBasePacks();

    void resetContent(
        EnginePaths& paths, Input& input, std::vector<ContentPack>& packs
    );

    void loadContent(
        EnginePaths& paths,
        Input& input,
        std::vector<ContentPack>& packs,
        const std::vector<std::string>& names
    );

    void loadContent(
        EnginePaths& paths,
        Input& input,
        std::vector<ContentPack>& packs
    );

    std::vector<io::path> getDefaultSources();
private:
    std::unique_ptr<Content> content;
    std::vector<std::string> basePacks;
    std::function<void()> postContent;
};
