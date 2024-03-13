#include "Lightmap.h"
#include <assert.h>

#include "../util/data_io.h"

void Lightmap::set(const Lightmap* lightmap) {
    set(lightmap->map);
}

void Lightmap::set(const light_t* map) {
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        this->map[i] = map[i];
    }
}

static_assert(sizeof(light_t) == 2, "replace dataio calls to new light_t");

ubyte* Lightmap::encode() const {
    ubyte* buffer = new ubyte[LIGHTMAP_DATA_LEN];
    for (uint i = 0; i < CHUNK_VOL; i+=2) {
        buffer[i/2] = ((map[i] >> 12) & 0xF) | ((map[i+1] >> 8) & 0xF0);
    }
    return buffer;
}

light_t* Lightmap::decode(ubyte* buffer) {
    light_t* lights = new light_t[CHUNK_VOL];
    for (uint i = 0; i < CHUNK_VOL; i+=2) {
        ubyte b = buffer[i/2];
        lights[i] = ((b & 0xF) << 12);
        lights[i+1] = ((b & 0xF0) << 8);
    } 
    return lights;
}
