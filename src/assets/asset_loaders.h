#ifndef ASSETS_ASSET_LOADERS_H_
#define ASSETS_ASSET_LOADERS_H_

#include <string>
#include <filesystem>

class Assets;

namespace assetload {
    bool texture(Assets* assets,
                 const std::filesystem::path filename,
                 const std::string name);
    bool shader(Assets* assets, 
                 const std::filesystem::path filename, 
                 const std::string name);
    bool atlas(Assets* assets, 
                const std::filesystem::path directory, 
                const std::string name);
    bool font(Assets* assets, 
                const std::filesystem::path filename, 
                const std::string name);
}

#endif // ASSETS_ASSET_LOADERS_H_