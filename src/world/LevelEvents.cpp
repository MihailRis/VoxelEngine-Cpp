#include "LevelEvents.hpp"
#include "../voxels/Chunk.hpp"

using std::vector;

void LevelEvents::listen(lvl_event_type type, chunk_event_func func) {
	auto& callbacks = chunk_callbacks[type];
	callbacks.push_back(func);
}

void LevelEvents::trigger(lvl_event_type type, Chunk* chunk) {
	const auto& callbacks = chunk_callbacks[type];
	for (chunk_event_func func : callbacks) {
		func(type, chunk);
	}
}
