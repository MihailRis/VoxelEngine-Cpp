#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>

class WorldFiles;
class Chunks;

class World {
public:
	std::string name;
	WorldFiles* wfile;

	World(std::string name, std::string directory);
	~World();
};

#endif /* WORLD_WORLD_H_ */
