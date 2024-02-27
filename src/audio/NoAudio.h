#ifndef AUDIO_NOAUDIO_H_
#define AUDIO_NOAUDIO_H_

#include "audio.h"

namespace audio {
    class NoSound : public Sound {
        std::shared_ptr<PCM> pcm;
        duration_t duration;
    public:
        NoSound(std::shared_ptr<PCM> pcm, bool keepPCM);
        ~NoSound() {}

        duration_t getDuration() const override {
            return duration;
        }

        std::shared_ptr<PCM> getPCM() const override {
            return pcm;
        }

        speakerid_t newInstance(int priority) const override {
            return 0;
        }
    };

    class NoAudio : public Backend {
    public:
        ~NoAudio() {}

        Sound* createSound(std::shared_ptr<PCM> pcm, bool keepPCM) override;

        void setListener(
            glm::vec3 position,
            glm::vec3 velocity,
            glm::vec3 at,
            glm::vec3 up
        ) override {}

        static NoAudio* create();
    };
}

#endif // AUDIO_NOAUDIO_H_
