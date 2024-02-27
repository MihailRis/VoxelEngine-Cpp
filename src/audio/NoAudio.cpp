#include "NoAudio.h"

using namespace audio;

NoSound::NoSound(std::shared_ptr<PCM> pcm, bool keepPCM) {
    duration = pcm->getDuration();
    if (keepPCM) {
        this->pcm = pcm;
    }
}

Sound* NoAudio::createSound(std::shared_ptr<PCM> pcm, bool keepPCM) {
    return new NoSound(pcm, keepPCM);
}

NoAudio* NoAudio::create() {
    return new NoAudio();
}
