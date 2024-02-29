#ifndef SRC_AUDIO_AUDIO_H_
#define SRC_AUDIO_AUDIO_H_

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <unordered_map>

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "../audio.h"
#include "../../typedefs.h"

namespace audio {
    struct ALBuffer;
    class ALAudio;
    class PCMStream;

    class ALSound : public Sound {
        ALAudio* al;
        uint buffer;
        std::shared_ptr<PCM> pcm;
        duration_t duration;
    public:
        ALSound(ALAudio* al, uint buffer, std::shared_ptr<PCM> pcm, bool keepPCM);
        ~ALSound();

        duration_t getDuration() const override {
            return duration;
        }

        std::shared_ptr<PCM> getPCM() const override {
            return pcm;
        }

        Speaker* newInstance(int priority) const override;
    };

    class ALStream : public Stream {
        ALAudio* al;
        std::shared_ptr<PCMStream> source;
        speakerid_t speaker = 0;
        bool keepSource;
    public:
        ALStream(ALAudio* al, std::shared_ptr<PCMStream> source, bool keepSource);
        ~ALStream();

        std::shared_ptr<PCMStream> getSource() const override;
        void bindSpeaker(speakerid_t speaker) override;
        Speaker* createSpeaker() override;
        speakerid_t getSpeaker() const override;
        void update(double delta) override;
        void setTime(duration_t time) override;        
    };

    /// @brief AL source adapter
    class ALSpeaker : public Speaker {
        ALAudio* al;
        uint source;
        int priority;
    public:
        ALSpeaker(ALAudio* al, uint source, int priority);
        ~ALSpeaker();

        State getState() const override;

        float getVolume() const override;
        void setVolume(float volume) override;

        float getPitch() const override;
        void setPitch(float pitch) override;

        bool isLoop() const override;
        void setLoop(bool loop) override;

        void play() override;
        void pause() override;
        void stop() override;

        duration_t getTime() const override;
        void setTime(duration_t time) override;

        void setPosition(glm::vec3 pos) override;
        glm::vec3 getPosition() const override;

        void setVelocity(glm::vec3 vel) override;
        glm::vec3 getVelocity() const override;

        int getPriority() const override;
    };

    class ALAudio : public Backend {
        ALCdevice* device;
        ALCcontext* context;

        std::vector<uint> allsources;
        std::vector<uint> freesources;

        std::vector<uint> allbuffers;
        std::vector<uint> freebuffers;

        uint maxSources;

        ALAudio(ALCdevice* device, ALCcontext* context);
    public:
        ~ALAudio();

        uint getFreeSource();
        uint getFreeBuffer();
        void freeSource(uint source);
        void freeBuffer(uint buffer);

        std::vector<std::string> getAvailableDevices() const;

        Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM) override;
        Stream* openStream(std::shared_ptr<PCMStream> stream, bool keepSource) override;

        void setListener(
            glm::vec3 position,
            glm::vec3 velocity,
            glm::vec3 lookAt,
            glm::vec3 up
        ) override;

        void update(double delta) override;
        
        bool isDummy() const override {
            return false;
        }

        static ALAudio* create();
    };
}

#endif /* SRC_AUDIO_AUDIO_H_ */
