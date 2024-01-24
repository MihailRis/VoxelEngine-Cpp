#ifndef LIGHTING_LIGHTING_H_
#define LIGHTING_LIGHTING_H_

class Content;
class ContentIndices;
class Chunk;
class Chunks;
class LightSolver;

class Lighting {
	const Content* const content;
	Chunks* chunks;
	LightSolver* solverR;
	LightSolver* solverG;
	LightSolver* solverB;
	LightSolver* solverS;
public:
	Lighting(const Content* content, Chunks* chunks);
	~Lighting();

	void clear();
	void buildSkyLight(int cx, int cz);
	void onChunkLoaded(int cx, int cz, bool expand);
	void onBlockSet(int x, int y, int z, int id);

	static void prebuildSkyLight(Chunk* chunk, const ContentIndices* indices);
};

#endif /* LIGHTING_LIGHTING_H_ */
