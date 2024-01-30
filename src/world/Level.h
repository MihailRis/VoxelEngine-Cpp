#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include <memory>

#include "../typedefs.h"
#include "../settings.h"
#include <list>

class Content;
class World;
class Player;
class Object;
class Chunks;
class LevelEvents;
class Lighting;
class PhysicsSolver;
class ChunksStorage;

class Level {
public:
	std::unique_ptr<World> world;
	const Content* const content;
	std::list<Object*> objects;
	Player* player;
	Chunks* chunks;
	ChunksStorage* chunksStorage;

	PhysicsSolver* physics;
	Lighting* lighting;
	LevelEvents* events;

	const EngineSettings& settings;

	Level(World* world, 
		  const Content* content,
	      EngineSettings& settings);
	~Level();

	void update();
    
    World* getWorld();

	template<class T, typename... Args>
	T* spawnObjectOfClass(Args&&... args);
};

#endif /* WORLD_LEVEL_H_ */
