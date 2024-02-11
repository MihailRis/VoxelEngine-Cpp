#ifndef ASSETS_ASSET_LOADERS_H_
#define ASSETS_ASSET_LOADERS_H_

#include <string>
#include <memory>

class ResPaths;
class Assets;
class Atlas;

namespace assetload {
    bool texture(
        Assets* assets, 
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );
    bool shader(
        Assets* assets,
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );
    bool atlas(
        Assets* assets,
        const ResPaths* paths, 
        const std::string directory, 
        const std::string name,
        std::shared_ptr<void> settings
    );
    bool font(
        Assets* assets, 
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );
    bool layout(
        Assets* assets,
        const ResPaths* paths,
        const std::string file,
        const std::string name,
        std::shared_ptr<void> settings
    );

    bool animation(
        Assets* assets,
        const ResPaths* paths,
        const std::string directory,
        const std::string name,
        Atlas* dstAtlas
    );
}

#endif // ASSETS_ASSET_LOADERS_H_