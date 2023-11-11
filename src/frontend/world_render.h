#ifndef WORLD_RENDERER_CPP
#define WORLD_RENDERER_CPP

#include <vector>
#include <algorithm>
#include <GL/glew.h>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

class World;
class Level;
class Camera;
class Assets;
class LineBatch;
class Batch3D;
class ChunksRenderer;
class Shader;
class Texture;
class Framebuffer;


class WorldRenderer {
	Batch3D* batch3d;
	Assets* assets;
	Level* level;
	bool drawChunk(size_t index, Camera* camera, Shader* shader, bool occlusion);
public:
	ChunksRenderer* renderer;
	LineBatch* lineBatch;

	WorldRenderer(Level* level, Assets* assets);
	~WorldRenderer();

	void draw(Camera* camera, bool occlusion, float fogFactor, float fogCurve);
};


#endif // WORLD_RENDERER_CPP
