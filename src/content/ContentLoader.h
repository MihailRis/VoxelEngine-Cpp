#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include <string>
#include <filesystem>

class Block;
class ContentBuilder;

class ContentLoader {
    std::filesystem::path folder;
public:
    ContentLoader(const std::filesystem::path& folder);

    Block* loadBlock(const std::string& name, const std::filesystem::path& file);
    void load(ContentBuilder* builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
