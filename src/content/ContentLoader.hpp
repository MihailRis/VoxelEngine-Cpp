#pragma once

#include <filesystem>
#include <memory>
#include <string>

#include "content_fwd.hpp"
#include "data/dv.hpp"

namespace fs = std::filesystem;

class Block;
struct BlockMaterial;
struct ItemDef;
struct EntityDef;
struct ContentPack;

class ContentBuilder;
class ContentPackRuntime;
struct ContentPackStats;

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

    static void loadCustomBlockModel(Block& def, const dv::value& primitives);
    static void loadBlockMaterial(BlockMaterial& def, const fs::path& file);
    void loadBlock(
        Block& def, const std::string& name, const fs::path& file
    );
    void loadItem(
        ItemDef& def, const std::string& name, const fs::path& file
    );
    void loadEntity(
        EntityDef& def, const std::string& name, const fs::path& file
    );
    void loadResources(ResourceType type, const dv::value& list);
public:
    ContentLoader(ContentPack* pack, ContentBuilder& builder);

    bool fixPackIndices(
        const fs::path& folder,
        dv::value& indicesRoot,
        const std::string& contentSection
    );
    void fixPackIndices();
    void load();
};
