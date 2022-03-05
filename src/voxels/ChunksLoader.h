#ifndef VOXELS_CHUNKSLOADER_H_
#define VOXELS_CHUNKSLOADER_H_

#include <thread>
#include <atomic>

class Chunk;

class ChunksLoader final {
private:
	std::thread loaderThread;
	void _thread();
	std::atomic<Chunk*> current {nullptr};
	std::atomic<Chunk**> closes {nullptr};
	std::atomic<bool> working {true};
public:
	ChunksLoader() : loaderThread{} {
		loaderThread = std::thread{&ChunksLoader::_thread, this};
	}
	~ChunksLoader(){
		working = false;
		loaderThread.join();
	}

	bool isBusy(){
		return current != nullptr;
	}

	void perform(Chunk* chunk, Chunk** closes_passed);

	void stop(){
		working = false;
	}
};

#endif /* VOXELS_CHUNKSLOADER_H_ */
