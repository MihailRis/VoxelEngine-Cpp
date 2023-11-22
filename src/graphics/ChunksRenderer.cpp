#include "ChunksRenderer.h"

#include "Mesh.h"
#include "BlocksRenderer.h"
#include "../voxels/Chunk.h"
#include "../world/Level.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::ivec2;
using std::shared_ptr;

ChunksRenderer::ChunksRenderer(Level* level, const ContentGfxCache* cache) : level(level) {
	const int MAX_FULL_CUBES = 3000;
	renderer = new BlocksRenderer(9 * 6 * 6 * MAX_FULL_CUBES, level->content, cache);
}

ChunksRenderer::~ChunksRenderer() {
	delete renderer;
}

shared_ptr<Mesh> ChunksRenderer::render(Chunk* chunk) {
	chunk->setModified(false);
	Mesh* mesh = renderer->render(chunk, 16, level->chunksStorage);
	auto sptr = shared_ptr<Mesh>(mesh);
	meshes[ivec2(chunk->x, chunk->z)] = sptr;
	return sptr;
}

void ChunksRenderer::unload(Chunk* chunk) {
	auto found = meshes.find(ivec2(chunk->x, chunk->z));
	if (found != meshes.end()) {
		meshes.erase(found);
	}
}

shared_ptr<Mesh> ChunksRenderer::getOrRender(Chunk* chunk) {
	auto found = meshes.find(ivec2(chunk->x, chunk->z));
	if (found != meshes.end() && !chunk->isModified()){
		return found->second;
	}
	return render(chunk);
}

shared_ptr<Mesh> ChunksRenderer::get(Chunk* chunk) {
	auto found = meshes.find(ivec2(chunk->x, chunk->z));
	if (found != meshes.end()) {
		return found->second;
	}
	return nullptr;
}
