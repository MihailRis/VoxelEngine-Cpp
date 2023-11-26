#ifndef FILES_ENGINE_PATHS_H_
#define FILES_ENGINE_PATHS_H_

#include <string>
#include <filesystem>

class EnginePaths {
    std::filesystem::path userfiles {"."};
    std::filesystem::path resources {"res"}; 
public:
    std::filesystem::path getUserfiles() const;
    std::filesystem::path getResources() const;
    
    std::filesystem::path getScreenshotFile(std::string ext);
    std::filesystem::path getWorldsFolder();
    bool isWorldNameUsed(std::string name);

    void setUserfiles(std::filesystem::path folder);
    void setResources(std::filesystem::path folder);
};

#endif // FILES_ENGINE_PATHS_H_