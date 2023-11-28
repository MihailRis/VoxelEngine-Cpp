#ifndef WORLD_WORLD_H_
#define WORLD_WORLD_H_

#include <string>
#include <filesystem>
#include "../typedefs.h"
#include "../settings.h"
#include "../util/timeutil.h"

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

	/* Day/night loop timer in range 0..1 
	   0.0 - is midnight
	   0.5 - is noon
	*/
	float daytime = timeutil::time_value(10, 00, 00);
	float daytimeSpeed = 1.0f/60.0f/24.0f;

	World(std::string name, 
		  std::filesystem::path directory, 
		  uint64_t seed, 
		  EngineSettings& settings);
	~World();

	void updateTimers(float delta);
	void write(Level* level);
	Level* load(EngineSettings& settings, const Content* content);
};

#endif /* WORLD_WORLD_H_ */