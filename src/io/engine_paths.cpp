#include "engine_paths.hpp"

#include <algorithm>
#include <array>
#include <filesystem>
#include <sstream>
#include <stack>
#include "typedefs.hpp"
#include "util/stringutil.hpp"
#include <utility>

#include "io/devices/StdfsDevice.hpp"
#include "world/files/WorldFiles.hpp"
#include "debug/Logger.hpp"

namespace fs = std::filesystem;

static debug::Logger logger("engine-paths");

static inline auto SCREENSHOTS_FOLDER = std::filesystem::u8path("screenshots");
static inline auto CONTENT_FOLDER = std::filesystem::u8path("content");
static inline auto WORLDS_FOLDER = std::filesystem::u8path("worlds");
static inline auto CONFIG_FOLDER = std::filesystem::u8path("config");
static inline auto EXPORT_FOLDER = std::filesystem::u8path("export");
static inline auto CONTROLS_FILE = std::filesystem::u8path("controls.toml");
static inline auto SETTINGS_FILE = std::filesystem::u8path("settings.toml");

static io::path toCanonic(io::path path) {
    std::stack<std::string> parts;
    
    path = std::filesystem::u8path(path.string()).lexically_normal().string();
    do {
        parts.push(path.name());
        path = path.parent();
    } while (!path.empty());

    path = "";

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
    io::set_device("res", std::make_shared<io::StdfsDevice>(resourcesFolder));
    io::set_device("user", std::make_shared<io::StdfsDevice>(userFilesFolder));

    if (!io::is_directory("res:")) {
        throw std::runtime_error(
            resourcesFolder.string() + " is not a directory"
        );
    }
    if (!io::is_directory("user:")) {
        io::create_directories("user:");
    }

    logger.info() << "resources folder: " << fs::canonical(resourcesFolder).u8string();
    logger.info() << "user files folder: " << fs::canonical(userFilesFolder).u8string();
    
    auto contentFolder = io::path("user:") / CONTENT_FOLDER;
    if (!io::is_directory(contentFolder)) {
        io::create_directories(contentFolder);
    }
    auto exportFolder = io::path("user:") / EXPORT_FOLDER;
    if (!io::is_directory(exportFolder)) {
        io::create_directories(exportFolder);
    }
    auto configFolder = io::path("user:") / CONFIG_FOLDER;
    if (!io::is_directory(configFolder)) {
        io::create_directories(configFolder);
    }
}

const std::filesystem::path& EnginePaths::getUserFilesFolder() const {
    return userFilesFolder;
}

const std::filesystem::path& EnginePaths::getResourcesFolder() const {
    return resourcesFolder;
}

io::path EnginePaths::getNewScreenshotFile(const std::string& ext) {
    auto folder = io::path("user:") / SCREENSHOTS_FOLDER;
    if (!io::is_directory(folder)) {
        io::create_directories(folder);
    }

    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);

    const char* format = "%Y-%m-%d_%H-%M-%S";
    std::stringstream ss;
    ss << std::put_time(&tm, format);
    std::string datetimestr = ss.str();

    auto file = folder / ("screenshot-" + datetimestr + "." + ext);
    uint index = 0;
    while (io::exists(file)) {
        file = folder / fs::u8path(
                            "screenshot-" + datetimestr + "-" +
                            std::to_string(index) + "." + ext
                        );
        index++;
    }
    return file;
}

io::path EnginePaths::getWorldsFolder() const {
    return io::path("user:") / WORLDS_FOLDER;
}

io::path EnginePaths::getConfigFolder() const {
    return io::path("user:") / CONFIG_FOLDER;
}

io::path EnginePaths::getCurrentWorldFolder() {
    return currentWorldFolder;
}

io::path EnginePaths::getWorldFolderByName(const std::string& name) {
    return getWorldsFolder() / std::filesystem::path(name);
}

io::path EnginePaths::getControlsFile() const {
    return io::path("user:") / CONTROLS_FILE;
}

io::path EnginePaths::getSettingsFile() const {
    return io::path("user:") / SETTINGS_FILE;
}

std::vector<io::path> EnginePaths::scanForWorlds() const {
    std::vector<io::path> folders;

    auto folder = getWorldsFolder();
    if (!io::is_directory(folder)) return folders;

    for (const auto& worldFolder : io::directory_iterator(folder)) {
        if (!io::is_directory(worldFolder)) {
            continue;
        }
        auto worldFile = worldFolder / WorldFiles::WORLD_FILE;
        if (!io::is_regular_file(worldFile)) {
            continue;
        }
        folders.push_back(worldFolder);
    }
    std::sort(
        folders.begin(),
        folders.end(),
        [](io::path a, io::path b) {
            a = a / WorldFiles::WORLD_FILE;
            b = b / WorldFiles::WORLD_FILE;
            return fs::last_write_time(io::resolve(a)) >
                   fs::last_write_time(io::resolve(b));
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

void EnginePaths::setScriptFolder(std::filesystem::path folder) {
    io::set_device("script", std::make_shared<io::StdfsDevice>(folder));
    this->scriptFolder = std::move(folder);
}

void EnginePaths::setCurrentWorldFolder(io::path folder) {
    this->currentWorldFolder = std::move(folder);
    io::create_subdevice("world", "user", currentWorldFolder);
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

// TODO: remove
io::path EnginePaths::resolve(
    const std::string& path, bool throwErr
) const {
    auto [prefix, filename] = EnginePaths::parsePath(path);
    if (prefix.empty()) {
        throw files_access_error("no entry point specified");
    }
    filename = toCanonic(filename).string();

    if (prefix == "core") {
        return io::path("res:") / filename;
    }

    if (prefix == "res" || prefix == "user" || prefix == "script") {
        return prefix + ":" + filename;
    }
    if (prefix == "config") {
        return getConfigFolder() / filename;
    }
    if (prefix == "world") {
        return currentWorldFolder / filename;
    }
    if (prefix == "export") {
        return io::path("user:") / EXPORT_FOLDER / filename;
    }
    if (contentPacks) {
        for (auto& pack : *contentPacks) {
            if (pack.id == prefix) {
                return pack.folder / filename;
            }
        }
    }
    if (throwErr) {
        throw files_access_error("unknown entry point '" + prefix + "'");
    }
    return filename;
}

ResPaths::ResPaths(io::path mainRoot, std::vector<PathsRoot> roots)
    : mainRoot(std::move(mainRoot)), roots(std::move(roots)) {
}

io::path ResPaths::find(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; i--) {
        auto& root = roots[i];
        auto file = root.path / filename;
        if (io::exists(file)) {
            return file;
        }
    }
    return mainRoot / filename;
}

std::string ResPaths::findRaw(const std::string& filename) const {
    for (int i = roots.size() - 1; i >= 0; i--) {
        auto& root = roots[i];
        if (io::exists(root.path / filename)) {
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
        if (!io::is_directory(folder)) continue;
        for (const auto& file : io::directory_iterator(folder)) {
            entries.emplace_back(root.name + ":" + folderName + "/" + file.name());
        }
    }
    return entries;
}

std::vector<io::path> ResPaths::listdir(
    const std::string& folderName
) const {
    std::vector<io::path> entries;
    for (int i = roots.size() - 1; i >= 0; i--) {
        auto& root = roots[i];
        io::path folder = root.path / folderName;
        if (!io::is_directory(folder)) continue;
        for (const auto& entry : io::directory_iterator(folder)) {
            entries.push_back(folder / entry);
        }
    }
    return entries;
}

dv::value ResPaths::readCombinedList(const std::string& filename) const {
    dv::value list = dv::list();
    for (const auto& root : roots) {
        auto path = root.path / filename;
        if (!io::exists(path)) {
            continue;
        }
        try {
            auto value = io::read_object(path);
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
        auto path = root.path / filename;
        if (!io::exists(path)) {
            continue;
        }
        try {
            auto value = io::read_object(path);
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

const io::path& ResPaths::getMainRoot() const {
    return mainRoot;
}
