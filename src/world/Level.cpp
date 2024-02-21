#include "Level.h"
#include "World.h"
#include "LevelEvents.h"
#include "../content/Content.h"
#include "../lighting/Lighting.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"
#include "../voxels/ChunksStorage.h"
#include "../physics/Hitbox.h"
#include "../physics/PhysicsSolver.h"
#include "../interfaces/Object.h"
#include "../objects/Player.h"
#include "../items/Inventory.h"
#include "../items/Inventories.h"


const float DEF_PLAYER_Y = 100.0f;
const float DEF_PLAYER_SPEED = 4.0f;
const int DEF_PLAYER_INVENTORY_SIZE = 40;

Level::Level(World* world, const Content* content, EngineSettings& settings)
	  : world(world),
	    content(content),
		chunksStorage(new ChunksStorage(this)),
		events(new LevelEvents()) ,
		settings(settings) 
{
	objCounter = 0;
    physics = new PhysicsSolver(glm::vec3(0, -22.6f, 0));
	auto inv = std::make_shared<Inventory>(0, DEF_PLAYER_INVENTORY_SIZE);
	player = spawnObject<Player>(glm::vec3(0, DEF_PLAYER_Y, 0), DEF_PLAYER_SPEED, inv);

    uint matrixSize = (settings.chunks.loadDistance+
					   settings.chunks.padding) * 2;
    chunks = new Chunks(matrixSize, matrixSize, 0, 0, 
						world->wfile, events, content);
	lighting = new Lighting(content, chunks);

	events->listen(EVT_CHUNK_HIDDEN, [this](lvl_event_type type, Chunk* chunk) {
		this->chunksStorage->remove(chunk->x, chunk->z);
	});

	inventories = std::make_unique<Inventories>(*this);
	inventories->store(player->getInventory());
}

Level::~Level(){
	delete chunks;
	delete events;
	delete physics;
	delete lighting;
	delete chunksStorage;

	for(auto obj : objects)
	{
		obj.reset();
	}
}

void Level::update() {
	glm::vec3 position = player->hitbox->position;
	chunks->setCenter(position.x, position.z);

	int matrixSize = (settings.chunks.loadDistance+
					  settings.chunks.padding) * 2;
	if (chunks->w != matrixSize) {
		chunks->resize(matrixSize, matrixSize);
	}
}

World* Level::getWorld() {
    return world.get();
}


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
