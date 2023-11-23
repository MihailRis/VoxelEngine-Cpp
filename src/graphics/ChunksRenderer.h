#ifndef SRC_GRAPHICS_CHUNKSRENDERER_H_
#define SRC_GRAPHICS_CHUNKSRENDERER_H_

#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "../voxels/Block.h"
#include "../voxels/ChunksStorage.h"
#include "../settings.h"

class Mesh;
class Chunk;
class Level;
class BlocksRenderer;
class ContentGfxCache;

class ChunksRenderer {
	BlocksRenderer* renderer;
	Level* level;
	std::unordered_map<glm::ivec2, std::shared_ptr<Mesh>> meshes;
public:
	ChunksRenderer(Level* level, 
				   const ContentGfxCache* cache, 
				   const EngineSettings& settings);
	virtual ~ChunksRenderer();

	std::shared_ptr<Mesh> render(Chunk* chunk);
	void unload(Chunk* chunk);

	std::shared_ptr<Mesh> getOrRender(Chunk* chunk);
	std::shared_ptr<Mesh> get(Chunk* chunk);

};

#endif // SRC_GRAPHICS_CHUNKSRENDERER_H_