#ifndef CONTENT_CONTENT_PACK_H_
#define CONTENT_CONTENT_PACK_H_

#include <string>
#include <vector>
#include <stdexcept>
#include <filesystem>

class EnginePaths;

class contentpack_error : public std::runtime_error {
    std::string packId;
    std::filesystem::path folder;
public:
    contentpack_error(std::string packId, 
                      std::filesystem::path folder, 
                      std::string message);

    std::string getPackId() const;
    std::filesystem::path getFolder() const;
};

struct ContentPack {
    std::string id = "none";
    std::string title = "untitled";
    std::string version = "0.0";
    std::filesystem::path folder;

    std::filesystem::path getContentFile() const;

    static const std::string PACKAGE_FILENAME;
    static const std::string CONTENT_FILENAME;
    static const std::filesystem::path BLOCKS_FOLDER;
    static const std::filesystem::path ITEMS_FOLDER;

    static bool is_pack(std::filesystem::path folder);
    static ContentPack read(std::filesystem::path folder);
    static void scan(std::filesystem::path folder, 
                     std::vector<ContentPack>& packs);
    static std::vector<std::string> worldPacksList(std::filesystem::path folder);

    static std::filesystem::path findPack(
        const EnginePaths* paths, 
        std::filesystem::path worldDir, 
        std::string name);
    static void readPacks(const EnginePaths* paths,
                          std::vector<ContentPack>& packs, 
                          const std::vector<std::string>& names,
                          std::filesystem::path worldDir);
};

#endif // CONTENT_CONTENT_PACK_H_
