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

    void loadBlock(Block* def, std::string full, std::string name);
    void loadCustomBlockModel(Block* def, json::JObject* primitives);
    void loadItem(ItemDef* def, std::string full, std::string name);
public:
    ContentLoader(ContentPack* pack);

    bool fixPackIndices(std::filesystem::path folder,
                        json::JObject* indicesRoot,
                        std::string contentSection);
    void fixPackIndices();
    void loadBlock(Block* def, std::string name, std::filesystem::path file);
    void loadItem(ItemDef* def, std::string name, std::filesystem::path file);
    void load(ContentBuilder* builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
