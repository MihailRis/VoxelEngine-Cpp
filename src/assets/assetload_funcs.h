#ifndef ASSETS_ASSET_LOADERS_H_
#define ASSETS_ASSET_LOADERS_H_

#include "Assets.h"

#include <string>
#include <memory>

class ResPaths;
class Assets;
class AssetsLoader;
class Atlas;
struct AssetCfg;

/// @brief see AssetsLoader.h: aloader_func
namespace assetload {
    postfunc texture(
        AssetsLoader*,
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<AssetCfg> settings
    );
    postfunc shader(
        AssetsLoader*,
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<AssetCfg> settings
    );
    postfunc atlas(
        AssetsLoader*,
        const ResPaths* paths, 
        const std::string directory, 
        const std::string name,
        std::shared_ptr<AssetCfg> settings
    );
    postfunc font(
        AssetsLoader*,
        const ResPaths* paths,
        const std::string filename, 
        const std::string name,
        std::shared_ptr<AssetCfg> settings
    );
    postfunc layout(
        AssetsLoader*,
        const ResPaths* paths,
        const std::string file,
        const std::string name,
        std::shared_ptr<AssetCfg> settings
    );

    postfunc sound(
        AssetsLoader*,
        const ResPaths* paths,
        const std::string file,
        const std::string name,
        std::shared_ptr<AssetCfg> settings
    );
}

#endif // ASSETS_ASSET_LOADERS_H_
