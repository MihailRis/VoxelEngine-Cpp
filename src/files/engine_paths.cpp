#include "engine_paths.h"

#include <filesystem>
#include <sstream>

#include "../typedefs.h"
#include "WorldFiles.h"

#define SCREENSHOTS_FOLDER "screenshots"

fs::path EnginePaths::getUserfiles() const {
    return userfiles;
}

fs::path EnginePaths::getResources() const {
    return resources;
}

fs::path EnginePaths::getScreenshotFile(std::string ext) {
    fs::path folder = userfiles/fs::path(SCREENSHOTS_FOLDER);
    if (!fs::is_directory(folder)) {
        fs::create_directory(folder);
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    const char* format = "%Y-%m-%d_%H-%M-%S";
    std::stringstream ss;
    ss << std::put_time(&tm, format);
    std::string datetimestr = ss.str();

    fs::path filename = folder/fs::path("screenshot-"+datetimestr+"."+ext);
    uint index = 0;
    while (fs::exists(filename)) {
        filename = folder/fs::path("screenshot-"+datetimestr+"-"+std::to_string(index)+"."+ext);
        index++;
    }
    return filename;
}

fs::path EnginePaths::getWorldsFolder() {
    return userfiles/fs::path("worlds");
}

fs::path EnginePaths::getWorldFolder() {
    return worldFolder;
}

std::vector<fs::path> EnginePaths::scanForWorlds() {
    std::vector<fs::path> folders;

    fs::path folder = getWorldsFolder();
    if (!fs::is_directory(folder))
        return folders;
    
    for (auto entry : fs::directory_iterator(folder)) {
        if (!entry.is_directory()) {
            continue;
        }
        fs::path worldFolder = entry.path();
        fs::path worldFile = worldFolder/fs::path(WorldFiles::WORLD_FILE);
        if (!fs::is_regular_file(worldFile)) {
            continue;
        }
        folders.push_back(worldFolder);
    }
    return folders;
}

bool EnginePaths::isWorldNameUsed(std::string name) {
    return fs::exists(EnginePaths::getWorldsFolder()/fs::u8path(name));
}

void EnginePaths::setUserfiles(fs::path folder) {
    this->userfiles = folder;
}

void EnginePaths::setResources(fs::path folder) {
    this->resources = folder;
}

void EnginePaths::setWorldFolder(fs::path folder) {
    this->worldFolder = folder;
}

void EnginePaths::setContentPacks(std::vector<ContentPack>* contentPacks) {
    this->contentPacks = contentPacks;
}

fs::path EnginePaths::resolve(std::string path) {
    size_t separator = path.find(':');
    if (separator == std::string::npos) {
        return fs::u8path(path);
    }
    std::string prefix = path.substr(0, separator);
    std::string filename = path.substr(separator+1);

    if (prefix == "res" || prefix == "core") {
        return resources/fs::u8path(filename);
    }

    if (prefix == "user") {
        return userfiles/fs::u8path(filename);
    }

    if (prefix == "world") {
        return worldFolder/fs::u8path(filename);
    }

    if (contentPacks) {
        for (auto& pack : *contentPacks) {
            if (pack.id == prefix) {
                return pack.folder/fs::u8path(filename);
            }
        }
    }
    return fs::u8path("./"+filename);
}

ResPaths::ResPaths(fs::path mainRoot, std::vector<fs::path> roots) 
    : mainRoot(mainRoot), roots(roots) {
}

fs::path ResPaths::find(const std::string& filename) const {
    for (int i = roots.size()-1; i >= 0; i--) {
        auto& root = roots[i];
        fs::path file = root / fs::path(filename);
        if (fs::exists(file)) {
            return file;
        }
    }
    return mainRoot / fs::path(filename);
}

std::vector<fs::path> ResPaths::listdir(const std::string& folderName) const {
    std::vector<fs::path> entries;
    for (int i = roots.size()-1; i >= 0; i--) {
        auto& root = roots[i];
        fs::path folder = root / fs::path(folderName);
        if (!fs::is_directory(folder))
            continue;
        for (const auto& entry : fs::directory_iterator(folder)) {
            entries.push_back(entry.path());
        }
    }
    {
        fs::path folder = mainRoot / fs::path(folderName);
        if (!fs::is_directory(folder))
            return entries;
        for (const auto& entry : fs::directory_iterator(folder)) {
            entries.push_back(entry.path());
        }
    }
    return entries;
}
