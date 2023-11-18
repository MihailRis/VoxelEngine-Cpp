#ifndef WORLD_RENDERER_CPP
#define WORLD_RENDERER_CPP

#include <vector>
#include <algorithm>
#include <GL/glew.h>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Level;
class Camera;
class LineBatch;
class ChunksRenderer;
class Shader;
class Texture;
class Frustum;
class Engine;

class WorldRenderer {
	Engine* engine;
	Level* level;
	Frustum* frustumCulling;
	LineBatch* lineBatch;
	ChunksRenderer* renderer;
	bool drawChunk(size_t index, Camera* camera, Shader* shader, bool occlusion);
public:

	WorldRenderer(Engine* engine, Level* level);
	~WorldRenderer();

	void draw(Camera* camera, bool occlusion);
};


#endif // WORLD_RENDERER_CPP
