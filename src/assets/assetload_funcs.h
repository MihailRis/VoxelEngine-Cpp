#ifndef ASSETS_ASSET_LOADERS_H_
#define ASSETS_ASSET_LOADERS_H_

#include <string>

class ResPaths;
class Assets;
class Atlas;

namespace assetload {
    bool texture(Assets* assets, 
                 const ResPaths* paths,
                 const std::string filename, 
                 const std::string name);
    bool shader(Assets* assets, 
                const ResPaths* paths,
                const std::string filename, 
                const std::string name);
    bool atlas(Assets* assets,
               const ResPaths* paths, 
               const std::string directory, 
               const std::string name);
    bool font(Assets* assets, 
              const ResPaths* paths,
              const std::string filename, 
              const std::string name);
	bool animation(Assets* assets,
		           const ResPaths* paths,
		           const std::string directory,
		           const std::string name,
		           Atlas* dstAtlas);
}

#endif // ASSETS_ASSET_LOADERS_H_