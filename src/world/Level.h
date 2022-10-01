#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

class World;
class Player;
class Chunks;
class Lighting;
class PhysicsSolver;
class ChunksController;
class PlayerController;

class Level {
public:
	Player* player;
	Chunks* chunks;
	PhysicsSolver* physics;
	Lighting* lighting;
	ChunksController* chunksController;
	PlayerController* playerController;
	Level(World* world, Player* player, Chunks* chunks, PhysicsSolver* physics);
	~Level();
};

#endif /* WORLD_LEVEL_H_ */
