#ifndef LIGHTING_LIGHTING_H_
#define LIGHTING_LIGHTING_H_

#include "../typedefs.h"

class Content;
class ContentIndices;
class Chunk;
class Chunks;
class ChunksStorage;
class LightSolver;

class Lighting {
	const Content* const content;
	Chunks* chunks;
	ChunksStorage* chunksStorage;
	std::unique_ptr<LightSolver> solverR;
	std::unique_ptr<LightSolver> solverG;
	std::unique_ptr<LightSolver> solverB;
	std::unique_ptr<LightSolver> solverS;
public:
	Lighting(const Content* content, Chunks* chunks, ChunksStorage* chunksStorage);
	~Lighting();

	void clear();
	void buildSkyLight(int cx, int cz);
	void onChunkLoaded(int cx, int cz, bool expand);
	void onBlockSet(int x, int y, int z, blockid_t id);

	static void prebuildSkyLight(Chunk* chunk, const ContentIndices* indices);
};

#endif /* LIGHTING_LIGHTING_H_ */
