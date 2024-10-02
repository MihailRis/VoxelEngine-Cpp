#pragma once

#include <filesystem>
#include <glm/glm.hpp>
#include <optional>
#include <memory>
#include <string>
#include <vector>

#include "content/ContentPack.hpp"
#include "typedefs.hpp"
#include "voxels/Chunk.hpp"
#include "WorldRegions.hpp"
#include "files.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

inline constexpr uint WORLD_FORMAT_VERSION = 1;

class Player;
class Content;
class ContentIndices;
class World;
struct WorldInfo;
struct DebugSettings;

namespace fs = std::filesystem;

class WorldFiles {
    fs::path directory;
    WorldRegions regions;

    bool generatorTestMode = false;
    bool doWriteLights = true;

    fs::path getWorldFile() const;
    fs::path getPacksFile() const;

    void writeWorldInfo(const WorldInfo& info);
    void writeIndices(const ContentIndices* indices);
public:
    WorldFiles(const fs::path& directory);
    WorldFiles(const fs::path& directory, const DebugSettings& settings);
    ~WorldFiles();

    fs::path getPlayerFile() const;
    fs::path getIndicesFile() const;
    fs::path getResourcesFile() const;
    void createDirectories();

    std::optional<WorldInfo> readWorldInfo();
    bool readResourcesData(const Content* content);

    static void createContentIndicesCache(
        const ContentIndices* indices, dv::value& root
    );
    static void createBlockFieldsIndices(
        const ContentIndices* indices, dv::value& root
    );

    void patchIndicesFile(const dv::value& map);

    /// @brief Write all unsaved data to world files
    /// @param world target world
    /// @param content world content
    void write(const World* world, const Content* content);

    void writePacks(const std::vector<ContentPack>& packs);

    void removeIndices(const std::vector<std::string>& packs);

    /// @return world folder
    fs::path getFolder() const;

    WorldRegions& getRegions() {
        return regions;
    }

    bool doesWriteLights() const {
        return doWriteLights;
    }

    static const inline std::string WORLD_FILE = "world.json";
};
