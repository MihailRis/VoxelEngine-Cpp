#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include <filesystem>
#include "../typedefs.h"
#include "../settings.h"

class Content;
class WorldFiles;
class Chunks;
class Level;
class Player;

class World {
public:
	std::string name;
	WorldFiles* wfile;
	uint64_t seed;

	World(std::string name, 
		  std::filesystem::path directory, 
		  uint64_t seed, 
		  EngineSettings& settings);
	~World();

	void write(Level* level);
	Level* load(EngineSettings& settings, const Content* content);
};

#endif /* WORLD_WORLD_H_ */