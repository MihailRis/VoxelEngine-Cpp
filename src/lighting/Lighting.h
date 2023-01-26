#ifndef LIGHTING_LIGHTING_H_
#define LIGHTING_LIGHTING_H_

class Chunks;
class LightSolver;

class Lighting {
	Chunks* chunks = nullptr;
	LightSolver* solverR = nullptr;
	LightSolver* solverG = nullptr;
	LightSolver* solverB = nullptr;
	LightSolver* solverS = nullptr;
public:
	Lighting(Chunks* chunks);
	~Lighting();

	void clear();
	void prebuildSkyLight(int cx, int cz);
	void buildSkyLight(int cx, int cz);
	void onChunkLoaded(int cx, int cz);
	void onBlockSet(int x, int y, int z, int id);
};

#endif /* LIGHTING_LIGHTING_H_ */
