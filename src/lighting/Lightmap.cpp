#include "Lightmap.h"

Lightmap::Lightmap(){
	map = new unsigned short[CHUNK_VOL];
	for (unsigned int i = 0; i < CHUNK_VOL; i++){
		map[i] = 0x0000;
	}
}

Lightmap::~Lightmap(){
	delete[] map;
}
