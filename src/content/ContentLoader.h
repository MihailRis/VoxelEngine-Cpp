#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include <string>
#include <filesystem>

class Block;
class Item;
class ContentBuilder;

class ContentLoader {
    std::filesystem::path folder;
public:
    ContentLoader(std::filesystem::path folder);

    Block* loadBlock(std::string name, std::filesystem::path file);
    Item* loadItem(std::string name, std::filesystem::path file);
    void load(ContentBuilder* builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
