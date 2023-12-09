#ifndef ASSETS_ASSETS_LOADER_H
#define ASSETS_ASSETS_LOADER_H

#include <string>
#include <functional>
#include <filesystem>
#include <map>
#include <queue>

const short ASSET_TEXTURE = 1;
const short ASSET_SHADER = 2;
const short ASSET_FONT = 3;
const short ASSET_ATLAS = 4;

class Assets;

typedef std::function<bool(Assets*, const std::filesystem::path&, const std::string&)> aloader_func;

struct aloader_entry {
	int tag;
	const std::filesystem::path filename;
	const std::string alias;
};

class AssetsLoader {
	Assets* assets;
	std::map<int, aloader_func> loaders;
	std::queue<aloader_entry> entries;
	std::filesystem::path resdir;
public:
	AssetsLoader(Assets* assets, const std::filesystem::path& resdir);
	void addLoader(int tag, const aloader_func& func);
	void add(int tag, const std::filesystem::path& filename, const std::string& alias);

	bool hasNext() const;
	bool loadNext();

	static void createDefaults(AssetsLoader& loader);
	static void addDefaults(AssetsLoader& loader);

	std::filesystem::path getDirectory() const;
};

#endif // ASSETS_ASSETS_LOADER_H
