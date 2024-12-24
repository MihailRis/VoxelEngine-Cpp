#pragma once

#include <filesystem>
#include <stdexcept>
#include <optional>
#include <string>
#include <vector>
#include <tuple>

#include "data/dv.hpp"
#include "content/ContentPack.hpp"


class files_access_error : public std::runtime_error {
public:
    files_access_error(const std::string& msg) : std::runtime_error(msg) {
    }
};

class EnginePaths {
public:
    void prepare();

    void setUserFilesFolder(std::filesystem::path folder);
    std::filesystem::path getUserFilesFolder() const;

    void setResourcesFolder(std::filesystem::path folder);
    std::filesystem::path getResourcesFolder() const;

    void setScriptFolder(std::filesystem::path folder);

    std::filesystem::path getWorldFolderByName(const std::string& name);
    std::filesystem::path getWorldsFolder() const;
    std::filesystem::path getConfigFolder() const;

    void setCurrentWorldFolder(std::filesystem::path folder);
    std::filesystem::path getCurrentWorldFolder();
    
    std::filesystem::path getNewScreenshotFile(const std::string& ext);
    std::filesystem::path getControlsFile() const;
    std::filesystem::path getSettingsFile() const;

    void setContentPacks(std::vector<ContentPack>* contentPacks);

    std::vector<std::filesystem::path> scanForWorlds() const;

    std::filesystem::path resolve(const std::string& path, bool throwErr = true) const;

    static std::tuple<std::string, std::string> parsePath(std::string_view view);

    static inline auto CONFIG_DEFAULTS =
        std::filesystem::u8path("config/defaults.toml");
private:
    std::filesystem::path userFilesFolder {"."};
    std::filesystem::path resourcesFolder {"res"};
    std::filesystem::path currentWorldFolder;
    std::optional<std::filesystem::path> scriptFolder;
    std::vector<ContentPack>* contentPacks = nullptr;
};

struct PathsRoot {
    std::string name;
    std::filesystem::path path;
};

class ResPaths {
public:
    ResPaths(std::filesystem::path mainRoot, std::vector<PathsRoot> roots);

    std::filesystem::path find(const std::string& filename) const;
    std::string findRaw(const std::string& filename) const;
    std::vector<std::filesystem::path> listdir(const std::string& folder) const;
    std::vector<std::string> listdirRaw(const std::string& folder) const;

    /// @brief Read all found list versions from all packs and combine into a
    /// single list. Invalid versions will be skipped with logging a warning
    /// @param file *.json file path relative to entry point 
    dv::value readCombinedList(const std::string& file) const;

    dv::value readCombinedObject(const std::string& file) const;

    const std::filesystem::path& getMainRoot() const;

private:
    std::filesystem::path mainRoot;
    std::vector<PathsRoot> roots;
};
