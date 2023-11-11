#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include "../typedefs.h"

class World;
class Player;
class Chunks;
class LevelEvents;
class Lighting;
class PhysicsSolver;
class ChunksController;
class ChunksStorage;
class PlayerController;

class Level {
public:
	World* world;
	Player* player;
	Chunks* chunks;
	ChunksStorage* chunksStorage;
	PhysicsSolver* physics;
	Lighting* lighting;
	ChunksController* chunksController;
	PlayerController* playerController;
	LevelEvents* events;
	Level(World* world, 
	      Player* player, 
	      ChunksStorage* chunksStorage,
	      LevelEvents* events,
	      uint loadDistance,
	      uint chunksPadding);
	~Level();

	void update(float delta, bool updatePlayer, bool interactions);
};

#endif /* WORLD_LEVEL_H_ */
