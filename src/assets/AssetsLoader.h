#ifndef ASSETS_ASSETS_LOADER_H
#define ASSETS_ASSETS_LOADER_H

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <queue>

const short ASSET_TEXTURE = 1;
const short ASSET_SHADER = 2;
const short ASSET_FONT = 3;
const short ASSET_ATLAS = 4;
const short ASSET_LAYOUT = 5;

class ResPaths;
class Assets;
class Content;

using aloader_func = std::function<bool(Assets*, const ResPaths*, const std::string&, const std::string&, std::shared_ptr<void>)>;

struct aloader_entry {
	int tag;
	const std::string filename;
	const std::string alias;
    std::shared_ptr<void> config;
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
        std::shared_ptr<void> settings=nullptr
    );

	bool hasNext() const;
	bool loadNext();

	static void addDefaults(AssetsLoader& loader, const Content* content);

	const ResPaths* getPaths() const;
};

#endif // ASSETS_ASSETS_LOADER_H
