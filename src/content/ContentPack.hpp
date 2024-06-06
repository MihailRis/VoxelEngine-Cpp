#ifndef CONTENT_CONTENT_PACK_HPP_
#define CONTENT_CONTENT_PACK_HPP_

#include "../typedefs.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>

class EnginePaths;

namespace fs = std::filesystem;

class contentpack_error : public std::runtime_error {
    std::string packId;
    fs::path folder;
public:
    contentpack_error(std::string packId, fs::path folder, std::string message);

    std::string getPackId() const;
    fs::path getFolder() const;
};

enum class DependencyLevel {
    required, // dependency must be installed
    optional, // dependency will be installed if found
    weak, // only affects packs order
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

    fs::path getContentFile() const;

    static const std::string PACKAGE_FILENAME;
    static const std::string CONTENT_FILENAME;
    static const fs::path BLOCKS_FOLDER;
    static const fs::path ITEMS_FOLDER;
    static const std::vector<std::string> RESERVED_NAMES;

    static bool is_pack(fs::path folder);
    static ContentPack read(fs::path folder);

    static void scanFolder(
        fs::path folder,
        std::vector<ContentPack>& packs
    );
    
    static std::vector<std::string> worldPacksList(fs::path folder);

    static fs::path findPack(
        const EnginePaths* paths, 
        fs::path worldDir, 
        std::string name
    );
};

struct ContentPackStats {
    size_t totalBlocks;
    size_t totalItems;

    inline bool hasSavingContent() const {
        return totalBlocks + totalItems > 0;
    }
};

class ContentPackRuntime {
    ContentPack info;
    ContentPackStats stats {};
    scriptenv env;
public:
    ContentPackRuntime(
        ContentPack info,
        scriptenv env
    );
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

#endif // CONTENT_CONTENT_PACK_HPP_
