#include "Lightmap.h"
#include <assert.h>

#include "../util/data_io.h"

void Lightmap::set(const Lightmap& lightmap) {
    map = lightmap.map;
}

void Lightmap::set(std::vector<light_t>&& map) {
    this->map = std::forward<std::vector<light_t>>(map);
}

void Lightmap::clear() {
    map.assign(map.size(), 0);
}

static_assert(sizeof(light_t) == 2, "replace dataio calls to new light_t");

std::vector<ubyte> Lightmap::encode() const {
	std::vector<ubyte> buffer(LIGHTMAP_DATA_LEN);
	for (uint i = 0; i < CHUNK_VOL; i+=2) {
		buffer[i/2] = ((map[i] >> 12) & 0xF) | ((map[i+1] >> 8) & 0xF0);
	}
	return buffer;
}

std::vector<light_t> Lightmap::decode(const std::vector<ubyte>& buffer) {
	assert(buffer.size() == LIGHTMAP_DATA_LEN);
	std::vector<light_t> lights(CHUNK_VOL);
	for (uint i = 0; i < CHUNK_VOL; i+=2) {
		ubyte b = buffer[i/2];
		lights[i] = ((b & 0xF) << 12);
		lights[i+1] = ((b & 0xF0) << 8);
	}
	return lights;
}
