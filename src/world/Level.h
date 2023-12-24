#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include "../typedefs.h"
#include "../settings.h"

class Content;
class World;
class Player;
class Chunks;
class LevelEvents;
class Lighting;
class PhysicsSolver;
class ChunksStorage;

class Level {
public:
	World* world;
	const Content* const content;
	Player* player;
	Chunks* chunks;
	ChunksStorage* chunksStorage;

	PhysicsSolver* physics;
	Lighting* lighting;
	LevelEvents* events;

	const EngineSettings& settings;

	Level(World* world, 
		  const Content* content,
	      Player* player, 
	      EngineSettings& settings);
	~Level();

	void update();
};

#endif /* WORLD_LEVEL_H_ */
