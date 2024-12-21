#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include "typedefs.hpp"
#include "content_fwd.hpp"

class EnginePaths;

namespace fs = std::filesystem;

class contentpack_error : public std::runtime_error {
    std::string packId;
    fs::path folder;
public:
    contentpack_error(
        std::string packId, fs::path folder, const std::string& message
    );

    std::string getPackId() const;
    fs::path getFolder() const;
};

enum class DependencyLevel {
    required,  // dependency must be installed
    optional,  // dependency will be installed if found
    weak,      // only affects packs order
};

/// @brief Content-pack that should be installed earlier the dependent
struct DependencyPack {
    DependencyLevel level;
    std::string id;
};

struct ContentPack {
    std::string id = "none";
    std::string title = "untitled";
    std::string version = "0.0";
    std::string creator = "";
    std::string description = "no description";
    fs::path folder;
    std::vector<DependencyPack> dependencies;
    std::string source = "";

    fs::path getContentFile() const;

    static inline const std::string PACKAGE_FILENAME = "package.json";
    static inline const std::string CONTENT_FILENAME = "content.json";
    static inline const fs::path BLOCKS_FOLDER = "blocks";
    static inline const fs::path ITEMS_FOLDER = "items";
    static inline const fs::path ENTITIES_FOLDER = "entities";
    static inline const fs::path GENERATORS_FOLDER = "generators";
    static const std::vector<std::string> RESERVED_NAMES;

    static bool is_pack(const fs::path& folder);
    static ContentPack read(const fs::path& folder);

    static void scanFolder(
        const fs::path& folder, std::vector<ContentPack>& packs
    );

    static std::vector<std::string> worldPacksList(const fs::path& folder);

    static fs::path findPack(
        const EnginePaths* paths,
        const fs::path& worldDir,
        const std::string& name
    );

    static ContentPack createCore(const EnginePaths&);

    static inline fs::path getFolderFor(ContentType type) {
        switch (type) {
            case ContentType::BLOCK: return ContentPack::BLOCKS_FOLDER;
            case ContentType::ITEM: return ContentPack::ITEMS_FOLDER;
            case ContentType::ENTITY: return ContentPack::ENTITIES_FOLDER;
            case ContentType::GENERATOR: return ContentPack::GENERATORS_FOLDER;
            case ContentType::NONE: return fs::u8path("");
            default: return fs::u8path("");
        }
    }
};

struct ContentPackStats {
    size_t totalBlocks;
    size_t totalItems;
    size_t totalEntities;

    inline bool hasSavingContent() const {
        return totalBlocks + totalItems + totalEntities > 0;
    }
};

struct WorldFuncsSet {
    bool onblockplaced;
    bool onblockreplaced;
    bool onblockbreaking;
    bool onblockbroken;
    bool onblockinteract;
    bool onplayertick;
};

class ContentPackRuntime {
    ContentPack info;
    ContentPackStats stats {};
    scriptenv env;
public:
    WorldFuncsSet worldfuncsset {};

    ContentPackRuntime(ContentPack info, scriptenv env);
    ~ContentPackRuntime();

    inline const ContentPackStats& getStats() const {
        return stats;
    }

    inline ContentPackStats& getStatsWriteable() {
        return stats;
    }

    inline const std::string& getId() {
        return info.id;
    }

    inline const ContentPack& getInfo() const {
        return info;
    }

    inline scriptenv getEnvironment() const {
        return env;
    }
};
