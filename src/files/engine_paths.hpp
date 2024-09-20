#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#include <tuple>

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

    std::filesystem::path getWorldsFolder();
    std::filesystem::path getWorldFolderByName(const std::string& name);

    void setCurrentWorldFolder(std::filesystem::path folder);
    std::filesystem::path getCurrentWorldFolder();
    
    std::filesystem::path getNewScreenshotFile(const std::string& ext);
    std::filesystem::path getControlsFile();
    std::filesystem::path getSettingsFile();

    void setContentPacks(std::vector<ContentPack>* contentPacks);

    std::vector<std::filesystem::path> scanForWorlds();

    std::filesystem::path resolve(const std::string& path, bool throwErr = true);

    static std::tuple<std::string, std::string> parsePath(std::string_view view);
private:
    std::filesystem::path userFilesFolder {"."};
    std::filesystem::path resourcesFolder {"res"};
    std::filesystem::path currentWorldFolder;
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

    const std::filesystem::path& getMainRoot() const;

private:
    std::filesystem::path mainRoot;
    std::vector<PathsRoot> roots;
};
