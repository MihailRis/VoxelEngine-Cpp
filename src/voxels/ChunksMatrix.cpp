#include "ChunksMatrix.h"
#include "Chunk.h"
#include "../world/Level.h"
#include "../world/LevelEvents.h"
#include "../content/Content.h"
#include "../logic/ChunksController.h"

#include "../maths/voxmaths.h"
#include <vector>

ChunksMatrix::ChunksMatrix(Level* level, uint32_t w, uint32_t d, int32_t ox, int32_t oz, const EngineSettings& settings)
		: contentIds(level->content->getIndices()),
          controller(std::make_unique<ChunksController>(level, this, settings.chunks.padding)),
          chunks(w*d),
          chunksSecond(w*d),
		  w(w), d(d), ox(ox), oz(oz), 
          settings(settings),
		  events(std::make_unique<LevelEvents>()) {
	volume = (size_t)w*(size_t)d;
	chunksCount = 0;
}

void ChunksMatrix::setCenter(int32_t x, int32_t z) {
	int cx = floordiv(x, CHUNK_W);
	int cz = floordiv(z, CHUNK_D);
	cx -= ox + w / 2;
	cz -= oz + d / 2;
	if (cx | cz) {
		translate(cx,cz);
	}
}

void ChunksMatrix::translate(int32_t dx, int32_t dz) {
	for (uint i = 0; i < volume; i++){
		chunksSecond[i] = nullptr;
	}
	for (uint32_t z = 0; z < d; z++){
		for (uint32_t x = 0; x < w; x++){
			auto chunk = chunks[z * w + x];
			int nx = x - dx;
			int nz = z - dz;
			if (chunk == nullptr)
				continue;
			if (nx < 0 || nz < 0 || nx >= int(w) || nz >= int(d)){
				events->trigger(EVT_CHUNK_HIDDEN, chunk.get());
                chunk->uses--;
				chunksCount--;
				continue;
			}
			chunksSecond[nz * w + nx] = chunk;
		}
	}
	std::swap(chunks, chunksSecond);

	ox += dx;
	oz += dz;
}

void ChunksMatrix::resize(uint32_t newW, uint32_t newD) {
	if (newW < w) {
		int delta = w - newW;
		translate(delta / 2, 0);
		translate(-delta, 0);
		translate(delta, 0);
	}
	if (newD < d) {
		int delta = d - newD;
		translate(0, delta / 2);
		translate(0, -delta);
		translate(0, delta);
	}
	const int newVolume = newW * newD;
    std::vector<std::shared_ptr<Chunk>> newChunks(newVolume);
    std::vector<std::shared_ptr<Chunk>> newChunksSecond(newVolume);
	for (int z = 0; z < int(d) && z < int(newD); z++) {
		for (int x = 0; x < int(w) && x < int(newW); x++) {
			newChunks[z * newW + x] = chunks[z * w + x];
		}
	}
    w = newW;
    d = newD;
    volume = newVolume;
    chunks = std::move(newChunks);
    chunksSecond = std::move(newChunksSecond);
}

void ChunksMatrix::update() {
    controller->update(settings.chunks.loadSpeed);
}

void ChunksMatrix::_setOffset(int32_t x, int32_t z) {
	ox = x;
	oz = z;
}

bool ChunksMatrix::putChunk(std::shared_ptr<Chunk> chunk) {
	int x = chunk->x;
	int z = chunk->z;
	x -= ox;
	z -= oz;
	if (x < 0 || z < 0 || x >= int(w) || z >= int(d))
		return false;
	chunks[z * w + x] = chunk;
    chunk->uses++;
	chunksCount++;
	return true;
}

void ChunksMatrix::clear(){
	for (size_t i = 0; i < volume; i++){
		Chunk* chunk = chunks[i].get();
		if (chunk) {
            chunk->uses--;
			events->trigger(EVT_CHUNK_HIDDEN, chunk);
		}
		chunks[i] = nullptr;
	}
	chunksCount = 0;
}

uint32_t ChunksMatrix::getPadding() const {
    return controller->padding;
}
