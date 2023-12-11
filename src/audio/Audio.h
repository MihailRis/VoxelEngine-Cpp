#ifndef SRC_AUDIO_AUDIO_H_
#define SRC_AUDIO_AUDIO_H_

#include <vector>
#include <string>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <glm/glm.hpp>


struct ALBuffer;

struct ALSource {
	ALuint id;
	ALSource(ALuint id) : id(id) {}

	bool isPlaying();
	bool setPosition(glm::vec3 position);
	bool setVelocity(glm::vec3 velocity);
	bool setBuffer(ALBuffer* buffer);
	bool setLoop(bool loop);
	bool setGain(float gain);
	bool setPitch(float pitch);
	bool play();
};

struct ALBuffer {
	ALuint id;
	ALBuffer(ALuint id) : id(id) {}
	bool load(int format, const char* data, int size, int freq);
};

class Audio {
	static ALCdevice* device;
	static ALCcontext* context;

	static std::vector<ALSource*> allsources;
	static std::vector<ALSource*> freesources;

	static std::vector<ALBuffer*> allbuffers;
	static std::vector<ALBuffer*> freebuffers;

	static unsigned maxSources;
	static unsigned maxBuffers;

public:
	static ALSource* getFreeSource();
	static ALBuffer* getFreeBuffer();
	static void freeSource(ALSource* source);
	static void freeBuffer(ALBuffer* buffer);

	static bool initialize();
	static void finalize();
	static bool get_available_devices(std::vector<std::string>& devicesVec);

	static void setListener(glm::vec3 position, glm::vec3 velocity, glm::vec3 at, glm::vec3 up);

};

#endif /* SRC_AUDIO_AUDIO_H_ */
