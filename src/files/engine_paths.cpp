#include "engine_paths.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <sstream>
#include <stack>
#include "typedefs.hpp"
#include "util/stringutil.hpp"
#include <utility>

#include "WorldFiles.hpp"
#include "debug/Logger.hpp"

static debug::Logger logger("engine-paths");

static inline auto SCREENSHOTS_FOLDER = std::filesystem::u8path("screenshots");
static inline auto CONTENT_FOLDER = std::filesystem::u8path("content");
static inline auto WORLDS_FOLDER = std::filesystem::u8path("worlds");
static inline auto CONFIG_FOLDER = std::filesystem::u8path("config");
static inline auto EXPORT_FOLDER = std::filesystem::u8path("export");
static inline auto CONTROLS_FILE = std::filesystem::u8path("controls.toml");
static inline auto SETTINGS_FILE = std::filesystem::u8path("settings.toml");

static std::filesystem::path toCanonic(std::filesystem::path path) {
    std::stack<std::string> parts;
    path = path.lexically_normal();
    do {
        parts.push(path.filename().u8string());
        path = path.parent_path();
    } while (!path.empty());

    path = fs::u8path("");

    while (!parts.empty()) {
        const std::string part = parts.top();
        parts.pop();
        if (part == ".") {
            continue;
        }
        if (part == "..") {
            throw files_access_error("entry point reached");
        }

        path = path / std::filesystem::path(part);
    }
    return path;
}

void EnginePaths::prepare() {
    if (!fs::is_directory(resourcesFolder)) {
        throw std::runtime_error(
            resourcesFolder.u8string() + " is not a directory"
        );
    }
    if (!fs::is_directory(userFilesFolder)) {
        fs::create_directories(userFilesFolder);
    }

    logger.info() << "resources folder: " << fs::canonical(resourcesFolder).u8string();
    logger.info() << "user files folder: " << fs::canonical(userFilesFolder).u8string();
    
    auto contentFolder = userFilesFolder / CONTENT_FOLDER;
    if (!fs::is_directory(contentFolder)) {
        fs::create_directories(contentFolder);
    }
    auto exportFolder = userFilesFolder / EXPORT_FOLDER;
    if (!fs::is_directory(exportFolder)) {
        fs::create_directories(exportFolder);
    }
    auto configFolder = userFilesFolder / CONFIG_FOLDER;
    if (!fs::is_directory(configFolder)) {
        fs::create_directories(configFolder);
    }
}

std::filesystem::path EnginePaths::getUserFilesFolder() const {
    return userFilesFolder;
}

std::filesystem::path EnginePaths::getResourcesFolder() const {
    return resourcesFolder;
}

std::filesystem::path EnginePaths::getNewScreenshotFile(const std::string& ext) {
    auto folder = userFilesFolder / SCREENSHOTS_FOLDER;
    if (!fs::is_directory(folder)) {
        fs::create_directory(folder);
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    const char* format = "%Y-%m-%d_%H-%M-%S";
    std::stringstream ss;
    ss << std::put_time(&tm, format);
    std::string datetimestr = ss.str();

    auto filename = folder / fs::u8path("screenshot-" + datetimestr + "." + ext);
    uint index = 0;
    while (fs::exists(filename)) {
        filename = folder / fs::u8path(
                                "screenshot-" + datetimestr + "-" +
                                std::to_string(index) + "." + ext
                            );
        index++;
    }
    return filename;
}

std::filesystem::path EnginePaths::getWorldsFolder() const {
    return userFilesFolder / WORLDS_FOLDER;
}

std::filesystem::path EnginePaths::getConfigFolder() const {
    return userFilesFolder / CONFIG_FOLDER;
}

std::filesystem::path EnginePaths::getCurrentWorldFolder() {
    return currentWorldFolder;
}

std::filesystem::path EnginePaths::getWorldFolderByName(const std::string& name) {
    return getWorldsFolder() / std::filesystem::path(name);
}

std::filesystem::path EnginePaths::getControlsFile() const {
    return userFilesFolder / CONTROLS_FILE;
}

std::filesystem::path EnginePaths::getSettingsFile() const {
    return userFilesFolder / SETTINGS_FILE;
}

std::vector<std::filesystem::path> EnginePaths::scanForWorlds() const {
    std::vector<std::filesystem::path> folders;

    auto folder = getWorldsFolder();
    if (!fs::is_directory(folder)) return folders;

    for (const auto& entry : fs::directory_iterator(folder)) {
        if (!entry.is_directory()) {
            continue;
        }
        const auto& worldFolder = entry.path();
        auto worldFile = worldFolder / fs::u8path(WorldFiles::WORLD_FILE);
        if (!fs::is_regular_file(worldFile)) {
            continue;
        }
        folders.push_back(worldFolder);
    }
    std::sort(
        folders.begin(),
        folders.end(),
        [](std::filesystem::path a, std::filesystem::path b) {
            a = a / fs::u8path(WorldFiles::WORLD_FILE);
            b = b / fs::u8path(WorldFiles::WORLD_FILE);
            return fs::last_write_time(a) > fs::last_write_time(b);
        }
    );
    return folders;
}

void EnginePaths::setUserFilesFolder(std::filesystem::path folder) {
    this->userFilesFolder = std::move(folder);
}

void EnginePaths::setResourcesFolder(std::filesystem::path folder) {
    this->resourcesFolder = std::move(folder);
}

void EnginePaths::setCurrentWorldFolder(std::filesystem::path folder) {
    this->currentWorldFolder = std::move(folder);
}

void EnginePaths::setContentPacks(std::vector<ContentPack>* contentPacks) {
    this->contentPacks = contentPacks;
}

std::tuple<std::string, std::string> EnginePaths::parsePath(std::string_view path) {
    size_t separator = path.find(':');
    if (separator == std::string::npos) {
        return {"", std::string(path)};
    }
    auto prefix = std::string(path.substr(0, separator));
    auto filename = std::string(path.substr(separator + 1));
    return {prefix, filename};
}

std::filesystem::path EnginePaths::resolve(
    const std::string& path, bool throwErr
) const {
    auto [prefix, filename] = EnginePaths::parsePath(path);
    if (prefix.empty()) {
        throw files_access_error("no entry point specified");
    }
    filename = toCanonic(fs::u8path(filename)).u8string();

    if (prefix == "res" || prefix == "core") {
        return resourcesFolder / fs::u8path(filename);
    }
    if (prefix == "user") {
        return userFilesFolder / fs::u8path(filename);
    }
    if (prefix == "config") {
        return getConfigFolder() / fs::u8path(filename);
    }
    if (prefix == "world") {
        return currentWorldFolder / fs::u8path(filename);
    }
    if (prefix == "export") {
        return userFilesFolder / EXPORT_FOLDER / fs::u8path(filename);
    }

    if (contentPacks) {
        for (auto& pack : *contentPacks) {
            if (pack.id == prefix) {
                return pack.folder / fs::u8path(filename);
            }
        }
    }
    if (throwErr) {
        throw files_access_error("unknown entry point '" + prefix + "'");
    }
    return std::filesystem::path(filename);
}

ResPaths::ResPaths(std::filesystem::path mainRoot, std::vector<PathsRoot> roots)
    : mainRoot(std::move(mainRoot)), roots(std::move(roots)) {
}

std::filesystem::path ResPaths::find(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; i--) {
        auto& root = roots[i];
        auto file = root.path / fs::u8path(filename);
        if (fs::exists(file)) {
            return file;
        }
    }
    return mainRoot / fs::u8path(filename);
}

std::string ResPaths::findRaw(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; i--) {
        auto& root = roots[i];
        if (fs::exists(root.path / std::filesystem::path(filename))) {
            return root.name + ":" + filename;
        }
    }
    throw std::runtime_error("could not to find file " + util::quote(filename));
}

std::vector<std::string> ResPaths::listdirRaw(const std::string& folderName) const {
    std::vector<std::string> entries;
    for (int i = roots.size() - 1; i >= 0; i--) {
        auto& root = roots[i];
        auto folder = root.path / fs::u8path(folderName);
        if (!fs::is_directory(folder)) continue;
        for (const auto& entry : fs::directory_iterator(folder)) {
            auto name = entry.path().filename().u8string();
            entries.emplace_back(root.name + ":" + folderName + "/" + name);
        }
    }
    return entries;
}

std::vector<std::filesystem::path> ResPaths::listdir(
    const std::string& folderName
) const {
    std::vector<std::filesystem::path> entries;
    for (int i = roots.size() - 1; i >= 0; i--) {
        auto& root = roots[i];
        std::filesystem::path folder = root.path / fs::u8path(folderName);
        if (!fs::is_directory(folder)) continue;
        for (const auto& entry : fs::directory_iterator(folder)) {
            entries.push_back(entry.path());
        }
    }
    return entries;
}

dv::value ResPaths::readCombinedList(const std::string& filename) const {
    dv::value list = dv::list();
    for (const auto& root : roots) {
        auto path = root.path / fs::u8path(filename);
        if (!fs::exists(path)) {
            continue;
        }
        try {
            auto value = files::read_object(path);
            if (!value.isList()) {
                logger.warning() << "reading combined list " << root.name << ":"
                    << filename << " is not a list (skipped)";
                continue;
            }
            for (const auto& elem : value) {
                list.add(elem);
            }
        } catch (const std::runtime_error& err) {
            logger.warning() << "reading combined list " << root.name << ":" 
                << filename << ": " << err.what();
        }
    }
    return list;
}

dv::value ResPaths::readCombinedObject(const std::string& filename) const {
    dv::value object = dv::object();
    for (const auto& root : roots) {
        auto path = root.path / fs::u8path(filename);
        if (!fs::exists(path)) {
            continue;
        }
        try {
            auto value = files::read_object(path);
            if (!value.isObject()) {
                logger.warning()
                    << "reading combined object " << root.name << ": "
                    << filename << " is not an object (skipped)";
            }
            for (const auto& [key, element] : value.asObject())  {
                object[key] = element;
            }
        } catch (const std::runtime_error& err) {
            logger.warning() << "reading combined object " << root.name << ":"
                             << filename << ": " << err.what();
        }
    }
    return object;
}

const std::filesystem::path& ResPaths::getMainRoot() const {
    return mainRoot;
}
