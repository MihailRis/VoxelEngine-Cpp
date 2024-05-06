#include "Lightmap.hpp"

#include "../util/data_io.hpp"

#include <assert.h>

void Lightmap::set(const Lightmap* lightmap) {
    set(lightmap->map);
}

void Lightmap::set(const light_t* map) {
    for (size_t i = 0; i < CHUNK_VOL; i++) {
        this->map[i] = map[i];
    }
}

static_assert(sizeof(light_t) == 2, "replace dataio calls to new light_t");

std::unique_ptr<ubyte[]> Lightmap::encode() const {
    auto buffer = std::make_unique<ubyte[]>(LIGHTMAP_DATA_LEN);
    for (uint i = 0; i < CHUNK_VOL; i+=2) {
        buffer[i/2] = ((map[i] >> 12) & 0xF) | ((map[i+1] >> 8) & 0xF0);
    }
    return buffer;
}

std::unique_ptr<light_t[]> Lightmap::decode(ubyte* buffer) {
    auto lights = std::make_unique<light_t[]>(CHUNK_VOL);
    for (uint i = 0; i < CHUNK_VOL; i+=2) {
        ubyte b = buffer[i/2];
        lights[i] = ((b & 0xF) << 12);
        lights[i+1] = ((b & 0xF0) << 8);
    } 
    return lights;
}
