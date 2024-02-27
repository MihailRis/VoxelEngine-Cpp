#ifndef SRC_AUDIO_AUDIO_H_
#define SRC_AUDIO_AUDIO_H_

#include <vector>
#include <string>
#include <glm/glm.hpp>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "audio.h"
#include "../typedefs.h"

namespace audio {
    struct ALBuffer;

    struct ALSource {
        uint id;
        ALSource(uint id) : id(id) {}

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
        uint id;
        ALBuffer(uint id) : id(id) {}
        bool load(int format, const char* data, int size, int freq);
    };

    class ALAudio : public Backend {
        ALCdevice* device;
        ALCcontext* context;

        std::vector<ALSource*> allsources;
        std::vector<ALSource*> freesources;

        std::vector<ALBuffer*> allbuffers;
        std::vector<ALBuffer*> freebuffers;

        uint maxSources;
        uint maxBuffers;

        ALAudio(ALCdevice* device, ALCcontext* context);
    public:
        ~ALAudio();

        ALSource* getFreeSource();
        ALBuffer* getFreeBuffer();
        void freeSource(ALSource* source);
        void freeBuffer(ALBuffer* buffer);

        std::vector<std::string> getAvailableDevices() const;

        Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM) override;

        void setListener(
            glm::vec3 position,
            glm::vec3 velocity,
            glm::vec3 lookAt,
            glm::vec3 up
        ) override;

        static ALAudio* create();
    };
}

#endif /* SRC_AUDIO_AUDIO_H_ */
