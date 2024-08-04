#ifndef CONTENT_PACKS_MANAGER_HPP_
#define CONTENT_PACKS_MANAGER_HPP_

#include <filesystem>
#include <unordered_map>
#include <vector>

#include "ContentPack.hpp"

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

    /// @brief Remove pack from manager to make it invisible for assembly(...)
    void exclude(const std::string& id);

    /// @brief Get all found packs
    std::vector<std::string> getAllNames() const;

    /// @brief Get packs by names (id)
    /// @param names pack names
    /// @throws contentpack_error if pack not found
    std::vector<ContentPack> getAll(const std::vector<std::string>& names
    ) const;

    /// @brief Resolve all dependencies and fix packs order
    /// @param names required packs (method can add extra packs)
    /// @return resulting ordered vector of pack names
    /// @throws contentpack_error if required dependency not found or
    /// circular dependency detected
    std::vector<std::string> assembly(const std::vector<std::string>& names
    ) const;

    /// @brief Collect all pack names (identifiers) into a new vector
    static std::vector<std::string> getNames(
        const std::vector<ContentPack>& packs
    );
};

#endif  // CONTENT_PACKS_MANAGER_HPP_
