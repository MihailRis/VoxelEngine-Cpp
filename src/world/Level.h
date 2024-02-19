#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include <memory>

#include "../typedefs.h"
#include "../settings.h"
#include <list>
#include <vector>
#include <chrono>

class Content;
class World;
class Player;
class Object;
class Chunks;
class Inventory;
class Inventories;
class LevelEvents;
class Lighting;
class PhysicsSolver;
class ChunksStorage;

class Level {
private:
	int objCounter;
public:
	std::unique_ptr<World> world;
	const Content* const content;
	std::list<std::shared_ptr<Object>> objects;
	std::shared_ptr<Player> player;
	Chunks* chunks;
	ChunksStorage* chunksStorage;
	std::unique_ptr<Inventories> inventories;

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
	std::shared_ptr<T> spawnObject(Args&&... args);
};

#endif /* WORLD_LEVEL_H_ */
