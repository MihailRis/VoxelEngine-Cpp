#ifndef CONTENT_CONTENT_PACK_H_
#define CONTENT_CONTENT_PACK_H_

#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>

class EnginePaths;

namespace fs = std::filesystem;

namespace scripting {
    class Environment;
}

class contentpack_error : public std::runtime_error {
    std::string packId;
    fs::path folder;
public:
    contentpack_error(std::string packId, 
                      fs::path folder, 
                      std::string message);

    std::string getPackId() const;
    fs::path getFolder() const;
};

struct ContentPack {
    std::string id = "none";
    std::string title = "untitled";
    std::string version = "0.0";
    std::string creator = "";
    std::string description = "no description";
    fs::path folder;
    std::vector<std::string> dependencies;

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

    static void scan(
        fs::path folder,
        EnginePaths* paths,
        std::vector<ContentPack>& packs
    );
    static void scan(
        EnginePaths* paths, 
        std::vector<ContentPack>& packs
    );
    
    static std::vector<std::string> worldPacksList(fs::path folder);

    static fs::path findPack(
        const EnginePaths* paths, 
        fs::path worldDir, 
        std::string name
    );

    static void readPacks(
        const EnginePaths* paths,
        std::vector<ContentPack>& packs, 
        const std::vector<std::string>& names,
        fs::path worldDir
    );
};

class ContentPackRuntime {
    ContentPack info;
    std::unique_ptr<scripting::Environment> env;
public:
    ContentPackRuntime(
        ContentPack info, 
        std::unique_ptr<scripting::Environment> env
    );

    inline const std::string& getId() {
        return info.id;
    }

    inline const ContentPack& getInfo() const {
        return info;
    }

    inline scripting::Environment* getEnvironment() const {
        return env.get();
    }
};

#endif // CONTENT_CONTENT_PACK_H_
