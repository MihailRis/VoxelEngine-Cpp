#ifndef ASSETS_ASSET_LOADERS_H_
#define ASSETS_ASSET_LOADERS_H_

#include <string>
#include <memory>

class ResPaths;
class Assets;
class AssetsLoader;
class Atlas;

namespace assetload {
    bool texture(
        AssetsLoader&,
        Assets*, 
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );
    bool shader(
        AssetsLoader&,
        Assets*,
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );
    bool atlas(
        AssetsLoader&,
        Assets*,
        const ResPaths* paths, 
        const std::string directory, 
        const std::string name,
        std::shared_ptr<void> settings
    );
    bool font(
        AssetsLoader&,
        Assets*, 
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<void> settings
    );
    bool layout(
        AssetsLoader&,
        Assets*,
        const ResPaths* paths,
        const std::string file,
        const std::string name,
        std::shared_ptr<void> settings
    );

    bool animation(
        Assets*,
        const ResPaths* paths,
        const std::string directory,
        const std::string name,
        Atlas* dstAtlas
    );
}

#endif // ASSETS_ASSET_LOADERS_H_