#ifndef ASSETS_ASSETS_LOADER_H
#define ASSETS_ASSETS_LOADER_H

#include <string>
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

typedef std::function<bool(Assets*, const ResPaths*, const std::string&, const std::string&)> aloader_func;

struct aloader_entry {
	int tag;
	const std::string filename;
	const std::string alias;
};

class AssetsLoader {
	Assets* assets;
	std::map<int, aloader_func> loaders;
	std::queue<aloader_entry> entries;
	const ResPaths* paths;
public:
	AssetsLoader(Assets* assets, const ResPaths* paths);
	void addLoader(int tag, aloader_func func);
	void add(int tag, const std::string filename, const std::string alias);

	bool hasNext() const;
	bool loadNext();

	static void createDefaults(AssetsLoader& loader);
	static void addDefaults(AssetsLoader& loader, bool world);

	const ResPaths* getPaths() const;
};

#endif // ASSETS_ASSETS_LOADER_H
