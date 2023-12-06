#include "Lightmap.h"
#include <assert.h>

#include "../util/data_io.h"

Lightmap::Lightmap(){
	map = new light_t[CHUNK_VOL];
	for (uint i = 0; i < CHUNK_VOL; i++){
		map[i] = 0x0000;
	}
}

Lightmap::~Lightmap(){
	delete[] map;
}

void Lightmap::set(const Lightmap* lightmap) {
	for (unsigned int i = 0; i < CHUNK_VOL; i++){
		map[i] = lightmap->map[i];
	}
}

static_assert(sizeof(light_t) == 2, "replace dataio calls to new light_t");

ubyte* Lightmap::encode() const {
	ubyte* buffer = new ubyte[CHUNK_VOL * sizeof(light_t)];
	for (uint i = 0; i < CHUNK_VOL; i++) {
		dataio::write_int16_big(map[i], buffer, i * sizeof(light_t));
	}
	return buffer;
}
