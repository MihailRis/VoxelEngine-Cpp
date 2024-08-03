#include "NoAudio.hpp"

using namespace audio;

NoSound::NoSound(const std::shared_ptr<PCM>& pcm, bool keepPCM) {
    duration = pcm->getDuration();
    if (keepPCM) {
        this->pcm = pcm;
    }
}

std::unique_ptr<Sound> NoAudio::createSound(
    std::shared_ptr<PCM> pcm, bool keepPCM
) {
    return std::make_unique<NoSound>(pcm, keepPCM);
}

std::unique_ptr<Stream> NoAudio::openStream(
    std::shared_ptr<PCMStream> stream, bool keepSource
) {
    return std::make_unique<NoStream>(stream, keepSource);
}

std::unique_ptr<NoAudio> NoAudio::create() {
    return std::make_unique<NoAudio>();
}
