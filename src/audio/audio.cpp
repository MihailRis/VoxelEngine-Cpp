#include "audio.h"

#include <iostream>

#include "ALAudio.h"
#include "NoAudio.h"

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

Sound* audio::createSound(std::shared_ptr<PCM> pcm, bool keepPCM) {
    return backend->createSound(pcm, keepPCM);
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
            speakers.erase(it);
            return;
        }
        it++;
    }
    for (auto it = speakers.begin(); it != speakers.end();) {
        if (it->second->getPriority() < priority) {
            it->second->stop();
            speakers.erase(it);
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
    speakers[id].reset(speaker);
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
            speakers.erase(it);
        } else {
            it++;
        }
    }
}

void audio::close() {
    delete backend;
    backend = nullptr;
}
