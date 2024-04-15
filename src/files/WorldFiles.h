#ifndef FILES_WORLD_FILES_H_
#define FILES_WORLD_FILES_H_

#include "WorldRegions.h"

#include "files.h"
#include "../typedefs.h"
#include "../settings.h"
#include "../content/ContentPack.h"
#include "../voxels/Chunk.h"

#include <vector>
#include <string>
#include <memory>
#include <filesystem>

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

inline constexpr uint WORLD_FORMAT_VERSION = 1;

class Player;
class Content;
class ContentIndices;
class World;

namespace fs = std::filesystem;

class WorldFiles {
    fs::path directory;
    WorldRegions regions;

    bool generatorTestMode = false;
    bool doWriteLights = true;

    fs::path getWorldFile() const;
    fs::path getIndicesFile() const;
    fs::path getPacksFile() const;

    void writeWorldInfo(const World* world);
    void writeIndices(const ContentIndices* indices);
public:
    WorldFiles(fs::path directory);
    WorldFiles(fs::path directory, const DebugSettings& settings);
    ~WorldFiles();

    fs::path getPlayerFile() const;
    void createDirectories();

    bool readWorldInfo(World* world);

    /// @brief Write all unsaved data to world files
    /// @param world target world
    /// @param content world content
    void write(const World* world, const Content* content);

    void writePacks(const std::vector<ContentPack>& packs);

    void removeIndices(const std::vector<std::string>& packs);

    /// @return world folder
    fs::path getFolder() const;

    static const inline std::string WORLD_FILE = "world.json";
    static bool parseRegionFilename(const std::string& name, int& x, int& y);

    WorldRegions& getRegions() {
        return regions;
    }
};

#endif // FILES_WORLD_FILES_H_
