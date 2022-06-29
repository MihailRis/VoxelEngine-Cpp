#include "World.h"

#include "../files/WorldFiles.h"
#include "../voxels/Chunk.h"
#include "../voxels/Chunks.h"

World::World(std::string name, std::string directory, Chunks* chunks) : name(name), chunks(chunks) {
	wfile = new WorldFiles(directory, REGION_VOL * (CHUNK_VOL * 2 + 8));
}

World::~World(){
	delete wfile;
}
