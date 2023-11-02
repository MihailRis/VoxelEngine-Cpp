#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

class World;
class Player;
class Chunks;
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
	Level(World* world, Player* player, Chunks* chunks, ChunksStorage* chunksStorage, PhysicsSolver* physics);
	~Level();

	void update(float delta, bool interactions);

};

#endif /* WORLD_LEVEL_H_ */
