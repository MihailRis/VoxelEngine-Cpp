#ifndef WORLD_RENDERER_CPP
#define WORLD_RENDERER_CPP

#include <vector>
#include <algorithm>
#include <GL/glew.h>
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../graphics/GfxContext.h"

class Level;
class Camera;
class LineBatch;
class ChunksRenderer;
class Shader;
class Texture;
class Frustum;
class Engine;
class Chunks;
class LevelFrontend;
class Skybox;

class WorldRenderer {
	Engine* engine;
	Level* level;
	std::unique_ptr<Frustum> frustumCulling;
	std::unique_ptr<LineBatch> lineBatch;
	std::unique_ptr<ChunksRenderer> renderer;
	std::unique_ptr<Skybox> skybox;
	bool drawChunk(size_t index, Camera* camera, Shader* shader, bool culling);
	void drawChunks(Chunks* chunks, Camera* camera, Shader* shader);
public:
	WorldRenderer(Engine* engine, LevelFrontend* frontend);
	~WorldRenderer();

	void draw(const GfxContext& context, Camera* camera, bool hudVisible);
	void drawDebug(const GfxContext& context, Camera* camera);
	void drawBorders(int sx, int sy, int sz, int ex, int ey, int ez);

	static float fog;
};


#endif // WORLD_RENDERER_CPP
