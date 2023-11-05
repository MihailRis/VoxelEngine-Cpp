#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include "../typedefs.h"

class WorldFiles;
class Chunks;
class Level;
class Player;

class World {
public:
	std::string name;
	WorldFiles* wfile;
	int seed;

	World(std::string name, std::string directory, int seed);
	~World();

	void write(Level* level);
	Level* loadLevel(Player* player, uint loadDistance, uint chunksPadding);
};

#endif /* WORLD_WORLD_H_ */
