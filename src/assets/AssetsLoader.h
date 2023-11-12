#ifndef ASSETS_ASSETS_LOADER_H
#define ASSETS_ASSETS_LOADER_H

#include <string>
#include <functional>
#include <map>
#include <queue>

#define ASSET_TEXTURE 1
#define ASSET_SHADER 2
#define ASSET_FONT 3
#define ASSET_ATLAS 4

class Assets;

typedef std::function<bool(Assets*, const std::string&, const std::string&)> aloader_func;

struct aloader_entry {
	int tag;
	const std::string filename;
	const std::string alias;
};

class AssetsLoader {
	Assets* assets;
	std::map<int, aloader_func> loaders;
	std::queue<aloader_entry> entries;
public:
	AssetsLoader(Assets* assets);
	void addLoader(int tag, aloader_func func);
	void add(int tag, const std::string filename, const std::string alias);

	bool hasNext() const;
	bool loadNext();

	static void createDefaults(AssetsLoader& loader);
	static void addDefaults(AssetsLoader& loader);
};

#endif // ASSETS_ASSETS_LOADER_H
