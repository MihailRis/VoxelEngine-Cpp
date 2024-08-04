#ifndef FILES_ENGINE_PATHS_HPP_
#define FILES_ENGINE_PATHS_HPP_

#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>

#include <content/ContentPack.hpp>

namespace fs = std::filesystem;

class files_access_error : public std::runtime_error {
public:
    files_access_error(const std::string& msg) : std::runtime_error(msg) {
    }
};

class EnginePaths {
    fs::path userfiles {"."};
    fs::path resources {"res"};
    fs::path worldFolder;
    std::vector<ContentPack>* contentPacks = nullptr;
public:
    void prepare();

    fs::path getUserfiles() const;
    fs::path getResources() const;

    fs::path getScreenshotFile(const std::string& ext);
    fs::path getWorldsFolder();
    fs::path getWorldFolder();
    fs::path getWorldFolder(const std::string& name);
    fs::path getControlsFile();
    fs::path getSettingsFile();
    bool isWorldNameUsed(const std::string& name);

    void setUserfiles(fs::path folder);
    void setResources(fs::path folder);
    void setContentPacks(std::vector<ContentPack>* contentPacks);
    void setWorldFolder(fs::path folder);

    std::vector<fs::path> scanForWorlds();

    fs::path resolve(const std::string& path, bool throwErr = true);
};

struct PathsRoot {
    std::string name;
    fs::path path;
};

class ResPaths {
    fs::path mainRoot;
    std::vector<PathsRoot> roots;
public:
    ResPaths(fs::path mainRoot, std::vector<PathsRoot> roots);

    fs::path find(const std::string& filename) const;
    std::string findRaw(const std::string& filename) const;
    std::vector<fs::path> listdir(const std::string& folder) const;
    std::vector<std::string> listdirRaw(const std::string& folder) const;

    const fs::path& getMainRoot() const;
};

#endif  // FILES_ENGINE_PATHS_HPP_
