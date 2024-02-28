#include "audio.h"

#include <iostream>
#include <stdexcept>

#include "NoAudio.h"
#include "AL/ALAudio.h"

#include "../coders/wav.h"
#include "../coders/ogg.h"

namespace audio {
    static speakerid_t nextId = 1;
    static Backend* backend;
    static std::unordered_map<speakerid_t, std::unique_ptr<Speaker>> speakers;
}

using namespace audio;

void audio::initialize(bool enabled) {
    if (enabled) {
        backend = ALAudio::create();
    }
    if (backend == nullptr) {
        std::cerr << "could not to initialize audio" << std::endl;
        backend = NoAudio::create();
    }
}

PCM* audio::loadPCM(const fs::path& file, bool headerOnly) {
    std::string ext = file.extension().u8string();
    if (ext == ".wav" || ext == ".WAV") {
        return wav::load_pcm(file, headerOnly);
    } else if (ext == ".ogg" || ext == ".OGG") {
        return ogg::load_pcm(file, headerOnly);
    }
    throw std::runtime_error("unsupported audio format");
}

Sound* audio::loadSound(const fs::path& file, bool keepPCM) {
    std::shared_ptr<PCM> pcm(loadPCM(file, !keepPCM && backend->isDummy()));
    return backend->createSound(pcm, keepPCM);
}

Sound* audio::createSound(std::shared_ptr<PCM> pcm, bool keepPCM) {
    return backend->createSound(pcm, keepPCM);
}

PCMStream* audio::openPCMStream(const fs::path& file) {
    std::string ext = file.extension().u8string();
    if (ext == ".ogg" || ext == ".OGG") {
        return ogg::create_stream(file);
    }
    throw std::runtime_error("unsupported audio stream format");
}

void audio::setListener(
    glm::vec3 position, 
    glm::vec3 velocity, 
    glm::vec3 lookAt, 
    glm::vec3 up
) {
    backend->setListener(position, velocity, lookAt, up);
}

void remove_lower_priority_speaker(int priority) {
    for (auto it = speakers.begin(); it != speakers.end();) {
        if (it->second->getPriority() < priority && it->second->isPaused()) {
            it->second->stop();
            it = speakers.erase(it);
            return;
        }
        it++;
    }
    for (auto it = speakers.begin(); it != speakers.end();) {
        if (it->second->getPriority() < priority) {
            it->second->stop();
            it = speakers.erase(it);
            return;
        }
        it++;
    }
}

speakerid_t audio::play(
    Sound* sound,
    glm::vec3 position,
    float volume,
    float pitch,
    bool loop,
    int priority
) {
    Speaker* speaker = sound->newInstance(priority);
    if (speaker == nullptr) {
        remove_lower_priority_speaker(priority);
        speaker = sound->newInstance(priority);
    }
    if (speaker == nullptr) {
        return 0;
    }
    speakerid_t id = nextId++;
    speakers.emplace(id, speaker);
    speaker->setPosition(position);
    speaker->setVolume(volume);
    speaker->setPitch(pitch);
    speaker->setLoop(loop);
    speaker->play();
    return id;
}

Speaker* audio::get(speakerid_t id) {
    auto found = speakers.find(id);
    if (found == speakers.end()) {
        return nullptr;
    }
    return found->second.get();
}

void audio::update(double delta) {
    backend->update(delta);

    for (auto it = speakers.begin(); it != speakers.end();) {
        if (it->second->isStopped()) {
            it = speakers.erase(it);
        } else {
            it++;
        }
    }
}

void audio::close() {
    speakers.clear();
    delete backend;
    backend = nullptr;
}
