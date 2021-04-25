#ifndef LIGHTING_LIGHTING_H_
#define LIGHTING_LIGHTING_H_

class Chunks;
class LightSolver;

class Lighting {
	static Chunks* chunks;
	static LightSolver* solverR;
	static LightSolver* solverG;
	static LightSolver* solverB;
	static LightSolver* solverS;
public:
	static int initialize(Chunks* chunks);
	static void finalize();

	static void clear();
	static void onChunkLoaded(int cx, int cy, int cz);
	static void onBlockSet(int x, int y, int z, int id);
};

#endif /* LIGHTING_LIGHTING_H_ */
