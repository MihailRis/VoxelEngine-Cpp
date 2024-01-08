#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include <string>
#include <filesystem>

class Block;
class ItemDef;
class ContentPack;
class ContentBuilder;

namespace json {
    class JObject;
}

class ContentLoader {
    const ContentPack* pack;

    Block* loadBlock(std::string name);
    ItemDef* loadItem(std::string name);
public:
    ContentLoader(ContentPack* pack);

    bool fixPackIndices(std::filesystem::path folder,
                        json::JObject* indicesRoot,
                        std::string contentSection);
    void fixPackIndices();
    Block* loadBlock(std::string name, std::filesystem::path file);
    ItemDef* loadItem(std::string name, std::filesystem::path file);
    void load(ContentBuilder* builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
