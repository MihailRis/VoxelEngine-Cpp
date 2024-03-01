#ifndef WORLD_LEVEL_H_
#define WORLD_LEVEL_H_

#include <memory>

#include "../typedefs.h"
#include "../interfaces/Object.h"
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

	// Spawns object of class T and returns pointer to it.
	// @param T class that derives the Object class
	// @param args pass arguments needed for T class constructor
	template<class T, typename... Args>
	std::shared_ptr<T> spawnObject(Args&&... args);
};


template<class T, typename... Args>
std::shared_ptr<T> Level::spawnObject(Args&&... args)
{
	static_assert(std::is_base_of<Object, T>::value, "T must be a derived of Object class");
	std::shared_ptr<T> tObj = std::make_shared<T>(args...);
	
	std::shared_ptr<Object> obj = std::dynamic_pointer_cast<Object, T>(tObj);
	objects.push_back(obj);
	obj->objectUID = objCounter;
	obj->spawned();
	objCounter += 1;
	return tObj;
}


#endif /* WORLD_LEVEL_H_ */
