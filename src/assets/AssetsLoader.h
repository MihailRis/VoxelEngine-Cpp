#ifndef ASSETS_ASSETS_LOADER_H
#define ASSETS_ASSETS_LOADER_H

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <queue>

inline constexpr short ASSET_TEXTURE = 1;
inline constexpr short ASSET_SHADER = 2;
inline constexpr short ASSET_FONT = 3;
inline constexpr short ASSET_ATLAS = 4;
inline constexpr short ASSET_LAYOUT = 5;
inline constexpr short ASSET_SOUND = 6;

class ResPaths;
class Assets;
class AssetsLoader;
class Content;

struct AssetCfg {
	virtual ~AssetCfg() {}
};

struct LayoutCfg : AssetCfg {
    int env;

    LayoutCfg(int env) : env(env) {}
};

struct SoundCfg : AssetCfg {
	bool keepPCM;
	
	SoundCfg(bool keepPCM) : keepPCM(keepPCM) {}
};

using aloader_func = std::function<bool(AssetsLoader&, Assets*, const ResPaths*, const std::string&, const std::string&, std::shared_ptr<AssetCfg>)>;

struct aloader_entry {
	int tag;
	const std::string filename;
	const std::string alias;
    std::shared_ptr<AssetCfg> config;
};

class AssetsLoader {
	Assets* assets;
	std::map<int, aloader_func> loaders;
	std::queue<aloader_entry> entries;
	const ResPaths* paths;
public:
	AssetsLoader(Assets* assets, const ResPaths* paths);
	void addLoader(int tag, aloader_func func);
	void add(
        int tag, 
        const std::string filename, 
        const std::string alias, 
        std::shared_ptr<AssetCfg> settings=nullptr
    );
	

	bool hasNext() const;
	bool loadNext();

	static void addDefaults(AssetsLoader& loader, const Content* content);

	const ResPaths* getPaths() const;
};

#endif // ASSETS_ASSETS_LOADER_H
