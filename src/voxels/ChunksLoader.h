#ifndef VOXELS_CHUNKSLOADER_H_
#define VOXELS_CHUNKSLOADER_H_

#ifdef _WIN32
#define _WIN32_WINNT 0x0501
#include <mingw.thread.h>
#else
#include <thread>
#endif

#include <atomic>

class Chunk;
class World;

enum LoaderMode {
	OFF, IDLE, LOAD, RENDER,
};

class ChunksLoader final {
private:
	std::thread loaderThread;
	void _thread();
	std::atomic<Chunk*> current {nullptr};
	std::atomic<Chunk**> closes {nullptr};
	std::atomic<LoaderMode> state {IDLE};
	World* world;

	void perform(Chunk* chunk, Chunk** closes_passed, LoaderMode mode);
public:
	ChunksLoader(World* world) : loaderThread{}, world(world) {
		loaderThread = std::thread{&ChunksLoader::_thread, this};
	}
	~ChunksLoader(){
		state = OFF;
		loaderThread.join();
	}

	bool isBusy(){
		return current != nullptr;
	}

	void load(Chunk* chunk, Chunk** closes_passed);
	void render(Chunk* chunk, Chunk** closes_passed);

	void stop(){
		state = OFF;
	}
};

#endif /* VOXELS_CHUNKSLOADER_H_ */
