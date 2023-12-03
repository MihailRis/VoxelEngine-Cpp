#ifndef WORLD_RENDERER_CPP
#define WORLD_RENDERER_CPP

#include <vector>
#include <algorithm>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../graphics/GfxContext.h"

class Level;
class Camera;
class LineBatch;
class ChunksRenderer;
class IShader;
class ITexture;
class Frustum;
class Engine;
class Chunks;
class ContentGfxCache;
class Skybox;

class WorldRenderer {
	Engine* engine;
	Level* level;
	Frustum* frustumCulling;
	LineBatch* lineBatch;
	ChunksRenderer* renderer;
	Skybox* skybox;
	bool drawChunk(size_t index, Camera* camera, IShader* shader, bool culling);
	void drawChunks(Chunks* chunks, Camera* camera, IShader* shader);
public:
	WorldRenderer(Engine* engine, Level* level, const ContentGfxCache* cache);
	~WorldRenderer();

	void draw(const GfxContext& context, Camera* camera);
	void drawDebug(const GfxContext& context, Camera* camera);
	void drawBorders(int sx, int sy, int sz, int ex, int ey, int ez);
};


#endif // WORLD_RENDERER_CPP
