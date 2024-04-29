#ifndef ASSETS_ASSETS_LOADER_H
#define ASSETS_ASSETS_LOADER_H

#include "Assets.h"
#include "../interfaces/Task.hpp"
#include "../typedefs.h"
#include "../delegates.h"

#include <string>
#include <memory>
#include <filesystem>
#include <functional>
#include <map>
#include <queue>

namespace dynamic {
	class Map;
	class List;
}

enum class AssetType {
	texture,
	shader,
	font,
	atlas,
	layout,
	sound
};

class ResPaths;
class AssetsLoader;
class Content;

struct AssetCfg {
	virtual ~AssetCfg() {}
};

struct LayoutCfg : AssetCfg {
    scriptenv env;

    LayoutCfg(scriptenv env) : env(env) {}
};

struct SoundCfg : AssetCfg {
	bool keepPCM;
	
	SoundCfg(bool keepPCM) : keepPCM(keepPCM) {}
};

using aloader_func = std::function<assetload::postfunc(
    AssetsLoader*, // redundant?
    const ResPaths*, 
    const std::string&, 
    const std::string&, 
    std::shared_ptr<AssetCfg>)
>;

struct aloader_entry {
	AssetType tag;
	const std::string filename;
	const std::string alias;
    std::shared_ptr<AssetCfg> config;
};

class AssetsLoader {
	Assets* assets;
	std::map<AssetType, aloader_func> loaders;
	std::queue<aloader_entry> entries;
	const ResPaths* paths;

    void tryAddSound(std::string name);

	void processPreload(AssetType tag, const std::string& name, dynamic::Map* map);
	void processPreloadList(AssetType tag, dynamic::List* list);
	void processPreloadConfig(std::filesystem::path file);
	void processPreloadConfigs(const Content* content);
public:
	AssetsLoader(Assets* assets, const ResPaths* paths);
	void addLoader(AssetType tag, aloader_func func);

	/// @brief Enqueue asset load
	/// @param tag asset type
	/// @param filename asset file path
	/// @param alias internal asset name
	/// @param settings asset loading settings (based on asset type)
	void add(
        AssetType tag, 
        const std::string filename, 
        const std::string alias, 
        std::shared_ptr<AssetCfg> settings=nullptr
    );
	
	bool hasNext() const;
	bool loadNext();

    std::shared_ptr<Task> startTask(runnable onDone);

	const ResPaths* getPaths() const;
    aloader_func getLoader(AssetType tag);

	/// @brief Enqueue core and content assets
	/// @param loader target loader
	/// @param content engine content
	static void addDefaults(AssetsLoader& loader, const Content* content);

    static bool loadExternalTexture(
        Assets* assets,
        const std::string& name,
        std::vector<std::filesystem::path> alternatives
    );
};

#endif // ASSETS_ASSETS_LOADER_H
