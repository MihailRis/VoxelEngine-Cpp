#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class Block;
class ItemDef;
struct ContentPack;
class ContentBuilder;

namespace dynamic {
    class Map;
}

class ContentLoader {
    const ContentPack* pack;
    int env = 0;

    void loadBlock(Block& def, std::string full, std::string name);
    void loadCustomBlockModel(Block& def, dynamic::Map* primitives);
    void loadItem(ItemDef& def, std::string full, std::string name);
public:
    ContentLoader(ContentPack* pack);

    bool fixPackIndices(std::filesystem::path folder,
                        dynamic::Map* indicesRoot,
                        std::string contentSection);
    void fixPackIndices();
    void loadBlock(Block& def, std::string name, fs::path file);
    void loadItem(ItemDef& def, std::string name, fs::path file);
    void load(ContentBuilder& builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
