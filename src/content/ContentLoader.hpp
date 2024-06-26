#ifndef CONTENT_CONTENT_LOADER_HPP_
#define CONTENT_CONTENT_LOADER_HPP_

#include "../typedefs.hpp"

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class Block;
struct BlockMaterial;
struct ItemDef;
struct ContentPack;
class ContentBuilder;

namespace dynamic {
    class Map;
}

class ContentLoader {
    const ContentPack* pack;
    scriptenv env;

    void loadBlock(Block& def, const std::string& full, const std::string& name);
    void loadCustomBlockModel(Block& def, dynamic::Map* primitives);
    void loadItem(ItemDef& def, const std::string& full, const std::string& name);
    void loadBlockMaterial(BlockMaterial& def, const fs::path& file);
public:
    ContentLoader(ContentPack* pack);

    bool fixPackIndices(
        const fs::path& folder,
        dynamic::Map* indicesRoot,
        const std::string& contentSection
    );
    void fixPackIndices();
    void loadBlock(Block& def, const std::string& name, const fs::path& file);
    void loadItem(ItemDef& def, const std::string& name, const fs::path& file);
    void load(ContentBuilder& builder);
};

#endif // CONTENT_CONTENT_LOADER_HPP_
