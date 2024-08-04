#ifndef CONTENT_CONTENT_LOADER_HPP_
#define CONTENT_CONTENT_LOADER_HPP_

#include <filesystem>
#include <memory>
#include <string>

#include "content_fwd.hpp"

namespace fs = std::filesystem;

class Block;
struct BlockMaterial;
struct ItemDef;
struct EntityDef;
struct ContentPack;

class ContentBuilder;
class ContentPackRuntime;
struct ContentPackStats;

namespace dynamic {
    class Map;
    class List;
}

class ContentLoader {
    const ContentPack* pack;
    ContentPackRuntime* runtime;
    scriptenv env;
    ContentBuilder& builder;
    ContentPackStats* stats;

    void loadBlock(
        Block& def, const std::string& full, const std::string& name
    );
    void loadItem(
        ItemDef& def, const std::string& full, const std::string& name
    );
    void loadEntity(
        EntityDef& def, const std::string& full, const std::string& name
    );

    static void loadCustomBlockModel(Block& def, dynamic::Map* primitives);
    static void loadBlockMaterial(BlockMaterial& def, const fs::path& file);
    static void loadBlock(
        Block& def, const std::string& name, const fs::path& file
    );
    static void loadItem(
        ItemDef& def, const std::string& name, const fs::path& file
    );
    static void loadEntity(
        EntityDef& def, const std::string& name, const fs::path& file
    );
    void loadResources(ResourceType type, dynamic::List* list);
public:
    ContentLoader(ContentPack* pack, ContentBuilder& builder);

    bool fixPackIndices(
        const fs::path& folder,
        dynamic::Map* indicesRoot,
        const std::string& contentSection
    );
    void fixPackIndices();
    void load();
};

#endif  // CONTENT_CONTENT_LOADER_HPP_
