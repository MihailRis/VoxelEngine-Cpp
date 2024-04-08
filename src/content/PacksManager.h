#ifndef CONTENT_PACKS_MANAGER_H_
#define CONTENT_PACKS_MANAGER_H_

#include "ContentPack.h"

#include <vector>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

class PacksManager {
    std::unordered_map<std::string, ContentPack> packs;
    std::vector<fs::path> sources;
public:
    PacksManager();

    /// @brief Set content packs sources (search folders)
    void setSources(std::vector<fs::path> sources);

    /// @brief Scan sources and collect all found packs excluding duplication.
    /// Scanning order depends on sources order
    void scan();

    /// @brief Get all found packs
    std::vector<std::string> getAllNames();

    /// @brief Resolve all dependencies and fix packs order
    /// @param names required packs (method can add extra packs)
    /// @return resulting ordered vector of pack names
    /// @throws contentpack_error if required dependency not found or
    /// circular dependency detected
    std::vector<std::string> assembly(const std::vector<std::string>& names);
};

#endif // CONTENT_PACKS_MANAGER_H_
