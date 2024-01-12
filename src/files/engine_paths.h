#ifndef FILES_ENGINE_PATHS_H_
#define FILES_ENGINE_PATHS_H_

#include <string>
#include <vector>
#include <filesystem>

class EnginePaths {
    std::filesystem::path userfiles {"."};
    std::filesystem::path resources {"res"}; 
public:
    std::filesystem::path getUserfiles() const;
    std::filesystem::path getResources() const;
    
    std::filesystem::path getScreenshotFile(const std::string& ext);
    std::filesystem::path getWorldsFolder();
    bool isWorldNameUsed(const std::string& name);

    void setUserfiles(const std::filesystem::path& folder);
    void setResources(const std::filesystem::path& folder);
};

class ResPaths {
    std::filesystem::path mainRoot;
    std::vector<std::filesystem::path> roots;
public:
    ResPaths(std::filesystem::path mainRoot,
             std::vector<std::filesystem::path> roots);
    
    std::filesystem::path find(const std::string& filename) const;
    std::vector<std::filesystem::path> listdir(const std::string& folder) const;
};

#endif // FILES_ENGINE_PATHS_H_