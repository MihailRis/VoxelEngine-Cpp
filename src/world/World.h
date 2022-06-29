#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>

class WorldFiles;
class Chunks;

class World {
public:
	std::string name;
	WorldFiles* wfile;
	Chunks* chunks;

	World(std::string name, std::string directory, Chunks* chunks);
	~World();
};

#endif /* WORLD_WORLD_H_ */
