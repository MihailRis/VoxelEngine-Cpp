#ifndef CONTENT_CONTENT_LOADER_H_
#define CONTENT_CONTENT_LOADER_H_

#include "../voxels/Block.h"

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

class ItemDef;
struct ContentPack;
class ContentBuilder;

namespace dynamic {
    class Map;
}

class ContentLoader {
    const ContentPack* pack;
    scriptenv env;

    void loadBlock(Block& def, std::string full, std::string name);
    void loadCustomBlockModel(Block& def, dynamic::Map* primitives);
    void loadItem(ItemDef& def, std::string full, std::string name);
    BlockMaterial loadBlockMaterial(fs::path file, std::string full);
public:
    ContentLoader(ContentPack* pack);

    bool fixPackIndices(
        fs::path folder,
        dynamic::Map* indicesRoot,
        std::string contentSection
    );
    void fixPackIndices();
    void loadBlock(Block& def, std::string name, fs::path file);
    void loadItem(ItemDef& def, std::string name, fs::path file);
    void load(ContentBuilder& builder);
};

#endif // CONTENT_CONTENT_LOADER_H_
