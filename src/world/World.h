#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include <filesystem>
#include "../typedefs.h"
#include "../settings.h"

class WorldFiles;
class Chunks;
class Level;
class Player;

class World {
public:
	std::string name;
	WorldFiles* wfile;
	int seed;

	World(std::string name, std::filesystem::path directory, int seed, EngineSettings& settings);
	~World();

	void write(Level* level, bool writeChunks);
	Level* loadLevel(EngineSettings& settings);
};

#endif /* WORLD_WORLD_H_ */