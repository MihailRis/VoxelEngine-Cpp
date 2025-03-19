#pragma once

#include <unordered_map>
#include <stdexcept>
#include <optional>
#include <string>
#include <vector>
#include <tuple>

#include "io.hpp"
#include "data/dv.hpp"
#include "content/ContentPack.hpp"

class EnginePaths {
public:
    void prepare();

    void setUserFilesFolder(std::filesystem::path folder);
    const std::filesystem::path& getUserFilesFolder() const;

    void setResourcesFolder(std::filesystem::path folder);
    const std::filesystem::path& getResourcesFolder() const;

    void setScriptFolder(std::filesystem::path folder);

    io::path getWorldFolderByName(const std::string& name);
    io::path getWorldsFolder() const;
    io::path getConfigFolder() const;

    void setCurrentWorldFolder(io::path folder);
    io::path getCurrentWorldFolder();
    
    io::path getNewScreenshotFile(const std::string& ext);
    io::path getControlsFile() const;
    io::path getSettingsFile() const;

    std::string createWriteablePackDevice(const std::string& name);

    void setContentPacks(std::vector<ContentPack>* contentPacks);

    std::vector<io::path> scanForWorlds() const;

    static std::tuple<std::string, std::string> parsePath(std::string_view view);

    static inline io::path CONFIG_DEFAULTS = "config/defaults.toml";
private:
    std::filesystem::path userFilesFolder {"."};
    std::filesystem::path resourcesFolder {"res"};
    io::path currentWorldFolder;
    std::optional<std::filesystem::path> scriptFolder;
    std::vector<ContentPack>* contentPacks = nullptr;
    std::vector<std::string> contentEntryPoints;
    std::unordered_map<std::string, std::string> writeablePacks;
};

struct PathsRoot {
    std::string name;
    io::path path;
};

class ResPaths {
public:
    ResPaths(io::path mainRoot, std::vector<PathsRoot> roots);

    io::path find(const std::string& filename) const;
    std::string findRaw(const std::string& filename) const;
    std::vector<io::path> listdir(const std::string& folder) const;
    std::vector<std::string> listdirRaw(const std::string& folder) const;

    /// @brief Read all found list versions from all packs and combine into a
    /// single list. Invalid versions will be skipped with logging a warning
    /// @param file *.json file path relative to entry point 
    dv::value readCombinedList(const std::string& file) const;

    dv::value readCombinedObject(const std::string& file, bool deep=false) const;

    const io::path& getMainRoot() const;

private:
    io::path mainRoot;
    std::vector<PathsRoot> roots;
};
